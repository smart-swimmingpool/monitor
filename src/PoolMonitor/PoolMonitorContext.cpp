// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file PoolMonitorContext.cpp
 * @brief PoolMonitorContext implementation — boot-loop detection, subsystem
 *        initialization, and the main monitoring loop.
 */

#include "PoolMonitorContext.hpp"

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <qrcode.h>

#include "Config.hpp"
#include "SystemMonitor.hpp"
#include "NetworkManager.hpp"
#include "DisplayManager.hpp"
#include "OtaUpdater.hpp"
#include "TimeClientHelper.hpp"
#include "../Version.h"

namespace PoolMonitor {

// Static member definitions
String PoolMonitorContext::mqtt_server;
uint16_t PoolMonitorContext::mqtt_server_port = 1883;

// Static context instance
static PoolMonitorContext *Self = nullptr;

// State variables
static float poolTemp_ = 0.0f;
static float solarTemp_ = 0.0f;
static bool poolPumpOn_ = false;
static bool solarPumpOn_ = false;
static String poolMode_ = "unknown";
static String lastUpdate_ = "??:??";

// Preferences for state persistence
static Preferences *preferences_ = nullptr;

// Home Assistant MQTT state topics for pool-controller (fixed, no discovery needed)

// Home Assistant MQTT state topics for pool-controller (fixed, no discovery needed)
const char* HA_TOPIC_POOL_TEMP = "homeassistant/sensor/pool-controller/pool-temp/state";
const char* HA_TOPIC_SOLAR_TEMP = "homeassistant/sensor/pool-controller/solar-temp/state";
const char* HA_TOPIC_POOL_PUMP = "homeassistant/switch/pool-controller/pool-pump/state";
const char* HA_TOPIC_SOLAR_PUMP = "homeassistant/switch/pool-controller/solar-pump/state";
const char* HA_TOPIC_MODE = "homeassistant/select/pool-controller/mode/state";

// Forward declaration
static void reconstructTime(unsigned long total_uptime);

PoolMonitorContext::PoolMonitorContext() {
  Self = this;
}

PoolMonitorContext::~PoolMonitorContext() {
  if (preferences_) {
    preferences_->end();
    delete preferences_;
    preferences_ = nullptr;
  }
  Self = nullptr;
}

// cppcheck-suppress unusedFunction ; called from main.cpp (cross-TU)
auto PoolMonitorContext::setup() -> void {
  Serial.println(F(" ------------------------------------- "));
  Serial.println(F("| Pool Monitor                        |"));
  Serial.println(F("| www.smart-swimmingpool.com          |"));
  Serial.println(F(" ------------------------------------- "));
  Serial.printf("📦\tFW Version: %s\n", FW_VERSION);
  Serial.printf("📦\tGitHub Repo: %s\n", GITHUB_REPO);

  // Initialize system monitor and check for boot loops
  SystemMonitor::begin();
  bootLoopDetected_ = SystemMonitor::detectBootLoop();

  // Initialize preferences
  preferences_ = new Preferences();
  if (!preferences_->begin("pool-monitor", false)) {
    Serial.println("🛑\tFailed to open preferences");
    while (1) {
      delay(1000);
    }
  }

  // Initialize OTA updater
  OtaUpdater::begin(*preferences_);

  // Track boot count
  unsigned int boot_count = preferences_->getUInt("boot_count", 0);
  Serial.printf("Current boot count: %u\n", ++boot_count);
  preferences_->putUInt("boot_count", boot_count);

  // Track cumulative uptime across sleep cycles
  unsigned long total_uptime = preferences_->getULong("total_uptime", 0);
  // Use actual sleep duration from last cycle (supports night-mode 4h sleeps)
  uint32_t lastSleepDuration = preferences_->getUInt("last_sleep_sec", TIME_TO_SLEEP_SECONDS);
  total_uptime += lastSleepDuration;
  preferences_->remove("last_sleep_sec");
  preferences_->putULong("total_uptime", total_uptime);
  Serial.printf("Total uptime: %lu seconds (%.1f hours, last sleep: %u s)\n",
                total_uptime, total_uptime / 3600.0, lastSleepDuration);

  // ── Power-save: WiFi/MQTT only every (SKIP_WIFI_WAKE_CYCLES + 1) wake-ups ──
  uint32_t cyclesWithoutWiFi = preferences_->getUInt("no_wifi_count", 0);

  // Force network on first boot (no MQTT config exists yet → portal needed)
  bool hasConfig = (preferences_->getString("mqtt_server", "").length() > 0);
  bool doNetwork = !hasConfig || (cyclesWithoutWiFi >= SKIP_WIFI_WAKE_CYCLES);

  // Scale no_wifi_count increment by actual sleep duration so a 4-hour
  // night sleep advances the counter by ~80 cycles (14400/180) instead
  // of only 1, preventing network-skip on the post-night wake.
  uint32_t skipIncrement = lastSleepDuration / TIME_TO_SLEEP_SECONDS;
  if (skipIncrement < 1) skipIncrement = 1;

  if (doNetwork) {
    preferences_->putUInt("no_wifi_count", 0);
  } else {
    preferences_->putUInt("no_wifi_count", cyclesWithoutWiFi + skipIncrement);
  }
  Serial.printf("📡\tNetwork cycle: %s (%u/%u without WiFi, last sleep: %u s, inc: %u)\n",
                doNetwork ? "YES" : "NO",
                doNetwork ? 0 : cyclesWithoutWiFi + 1,
                SKIP_WIFI_WAKE_CYCLES,
                lastSleepDuration,
                skipIncrement);

  // Initialize NTP time client
  PoolMonitor::beginTimeClient();

  // ── Boot-loop safe mode ──
  if (bootLoopDetected_) {
    Serial.println("⚠️\tBoot loop detected! Entering safe mode — skipping network, using cached data");
    // Reset counter so next wake tries normal boot instead of staying safe forever
    SystemMonitor::clearBootLoopCounter();
    initializeDisplay();
    loadState();
    reconstructTime(total_uptime);
    DisplayManager::updateDisplay(poolTemp_, solarTemp_, poolPumpOn_, solarPumpOn_,
                                  poolMode_.c_str(), lastUpdate_.c_str());
    prepareForSleep();  // deep sleep, does not return
  }

  // ── On network cycles: full display init + network + MQTT + display update ──
  // ── On no-network cycles: E-Ink retains its image, skip all display ops  ──
  if (doNetwork) {
    initializeDisplay();
    initializeNetwork();
    initializeMqtt();

    // Boot successful — reached after WiFi + MQTT without crash
    SystemMonitor::clearBootLoopCounter();

    // Load fresh data (MQTT callbacks may have updated NVS values)
    loadState();

    // NTP sync
    if (isNtpSyncNeeded()) {
      Serial.println("⏰\tNTP sync needed - updating time from server");
      lastUpdate_ = PoolMonitor::getCurrentTime();
      preferences_->putString("last_update", lastUpdate_);
      preferences_->putULong("last_ntp_sync", total_uptime);

      unsigned long synced_epoch = PoolMonitor::timeClient.getEpochTime();
      preferences_->putULong("last_epoch", synced_epoch);

      Serial.printf("⏰\tNTP synced successfully at %s (next sync in ~%d seconds)\n",
                    lastUpdate_.c_str(), NTP_SYNC_INTERVAL_SECONDS);
    } else {
      reconstructTime(total_uptime);
    }

    // Safety net: process any retained MQTT messages that arrived after polling
    NetworkManager::loop();
    // Reload state in case late messages updated preferences
    loadState();

    // Always update display on network cycles (data was potentially refreshed)
    Serial.println("🖥️\tUpdating display");
    DisplayManager::updateDisplay(poolTemp_, solarTemp_, poolPumpOn_, solarPumpOn_,
                                  poolMode_.c_str(), lastUpdate_.c_str());

    // Remember displayed values for next comparison
    preferences_->putFloat("last_display_pool", poolTemp_);
    preferences_->putFloat("last_display_solar", solarTemp_);
    preferences_->putBool("last_display_ppump", poolPumpOn_);
    preferences_->putBool("last_display_spump", solarPumpOn_);
    preferences_->putString("last_display_mode", poolMode_);

  } else {
    // No-network cycle: E-Ink retains image, skip all display operations
    // Only reconstruct time and load cached state for bookkeeping
    loadState();
    reconstructTime(total_uptime);

    // Load MQTT settings from preferences (for status display next cycle)
    mqtt_server = preferences_->getString("mqtt_server", "");
    mqtt_server_port = preferences_->getUInt("mqtt_port", 1883);

    Serial.println("🖥️\tNo network — E-Ink retains image, display skipped (saving power)");
  }
}

auto PoolMonitorContext::loop() -> void {
  // Feed watchdog
  SystemMonitor::feedWatchdog();

  // Check memory
  SystemMonitor::checkMemory();

  // Process MQTT messages
  NetworkManager::loop();

  // Process OTA update check (only when WiFi connected, saves power on skip cycles)
  unsigned long total_uptime = preferences_->getULong("total_uptime", 0);
  if (NetworkManager::isWiFiConnected() && OtaUpdater::checkForUpdate(total_uptime)) {
    Serial.println("⬆️\tOTA: New firmware available! Starting update...");
    if (!OtaUpdater::startUpdate()) {
      Serial.println("⚠️\tOTA: Update failed, will retry on next wake cycle");
    }
  }

  // Prepare for sleep
  prepareForSleep();
}

auto PoolMonitorContext::prepareForSleep() -> void {
  // ── Determine sleep interval ──
  uint32_t sleepSeconds = TIME_TO_SLEEP_SECONDS;

  unsigned long totalUptime = preferences_->getULong("total_uptime", 0);
  unsigned long lastEpoch = preferences_->getULong("last_epoch", 0);

  if (lastEpoch > 0) {
    // Reconstruct current local time to check if we're in the night window
    unsigned long lastNtpSync = preferences_->getULong("last_ntp_sync", 0);
    unsigned long elapsed = 0;
    if (totalUptime > lastNtpSync) {
      elapsed = totalUptime - lastNtpSync;
    }
    time_t t = PoolMonitor::currentTZ.toLocal(lastEpoch + elapsed);
    int currentHour = ::hour(t);
    int currentMinute = ::minute(t);
    int currentSecond = ::second(t);

    if (currentHour >= static_cast<int>(NIGHT_START_HOUR) ||
        currentHour < static_cast<int>(NIGHT_END_HOUR)) {
      sleepSeconds = NIGHT_SLEEP_INTERVAL_SECONDS;

      // Clamp night sleep so the device does not overshoot NIGHT_END_HOUR
      int secondsUntilEnd;
      if (currentHour >= static_cast<int>(NIGHT_START_HOUR)) {
        // Night started today (22:xx-23:xx), end is tomorrow 06:xx
        secondsUntilEnd = (static_cast<int>(NIGHT_END_HOUR) + 24 - currentHour) * 3600
                          - currentMinute * 60 - currentSecond;
      } else {
        // Night continues today (00:xx-05:xx), end is today 06:xx
        secondsUntilEnd = (static_cast<int>(NIGHT_END_HOUR) - currentHour) * 3600
                          - currentMinute * 60 - currentSecond;
      }

      if (secondsUntilEnd > 60 && sleepSeconds > static_cast<uint32_t>(secondsUntilEnd)) {
        sleepSeconds = secondsUntilEnd;
        Serial.printf("🌙\tClamping night sleep to %d sec (wake at %02d:00)\n",
                      sleepSeconds, NIGHT_END_HOUR);
      }
    }
  }

  Serial.printf("😴\tGoing to sleep now for %d sec.\n", sleepSeconds);

  // Save current state
  saveState();

  // Persist actual sleep duration for correct uptime tracking next boot
  preferences_->putUInt("last_sleep_sec", sleepSeconds);

  // Disconnect MQTT
  NetworkManager::disconnectMqtt();

  // Power down display
  DisplayManager::powerDown();

  // Close preferences
  preferences_->end();

  // Enter deep sleep
  esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000ULL);
  pinMode(PIN_MODEM_POWER_ON, OUTPUT);
  digitalWrite(PIN_MODEM_POWER_ON, LOW);

  esp_deep_sleep_start();
}

auto PoolMonitorContext::initializeDisplay() -> void {
  if (!DisplayManager::begin()) {
    Serial.println("🛑\tFailed to initialize display");
    return;
  }

  DisplayManager::initDisplay();
}

// Display callback for QR code
static struct {
  int16_t x, y;
  uint8_t scale;
} _qr_ctx;

/**
 * Display callback for esp_qrcode_generate — draws modules on the E-Ink display.
 */
static void _qrDisplayFunc(esp_qrcode_handle_t qrcode) {
  int size = esp_qrcode_get_size(qrcode);
  int quiet = 4 * _qr_ctx.scale;

  // Quiet zone (white border)
  DisplayManager::getDisplay().fillRect(_qr_ctx.x, _qr_ctx.y,
                                       size * _qr_ctx.scale + quiet * 2,
                                       size * _qr_ctx.scale + quiet * 2,
                                       GxEPD_WHITE);

  // Draw modules as black squares
  for (int qy = 0; qy < size; qy++) {
    for (int qx = 0; qx < size; qx++) {
      if (esp_qrcode_get_module(qrcode, qx, qy)) {
        DisplayManager::getDisplay().fillRect(_qr_ctx.x + quiet + qx * _qr_ctx.scale,
                                               _qr_ctx.y + quiet + qy * _qr_ctx.scale,
                                               _qr_ctx.scale, _qr_ctx.scale, GxEPD_BLACK);
      }
    }
  }
}

/**
 * Draw a QR code on the display at the given position.
 */
static void drawQrCode(const char* text, int16_t x, int16_t y, uint8_t scale) {
  _qr_ctx.x = x;
  _qr_ctx.y = y;
  _qr_ctx.scale = scale;

  esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
  cfg.display_func = _qrDisplayFunc;
  cfg.max_qrcode_version = 3;  // enough for ~55 bytes of alphanumeric data
  cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;

  if (esp_qrcode_generate(&cfg, text) != ESP_OK) {
    Serial.println("⚠️\tQR code generation failed");
  }
}

/**
 * @brief Reconstruct current time from last known epoch + elapsed uptime.
 * Used on no-network cycles or when NTP sync interval hasn't elapsed.
 */
static void reconstructTime(unsigned long total_uptime) {
  unsigned long last_epoch = preferences_->getULong("last_epoch", 0);
  unsigned long last_ntp_sync = preferences_->getULong("last_ntp_sync", 0);

  unsigned long elapsed_since_sync = 0;
  if (total_uptime > last_ntp_sync) {
    elapsed_since_sync = total_uptime - last_ntp_sync;
  }

  time_t t = PoolMonitor::currentTZ.toLocal(last_epoch + elapsed_since_sync);
  char buf[10];
  snprintf(buf, sizeof(buf), "%.2d:%.2d", hour(t), minute(t));
  lastUpdate_ = String(buf);
  preferences_->putString("last_update", lastUpdate_);
}

void PoolMonitorContext::showSetupScreen() {
  Serial.println("⚙️\tConfiguration portal active");

  // Build the WiFi QR code string
  String qrContent = "WIFI:T:nopass;S:" + String(DEVICE_NAME) + ";;";

  String apIP = WiFi.softAPIP().toString();

  DisplayManager::getDisplay().fillScreen(GxEPD_WHITE);
  DisplayManager::getDisplay().setRotation(3);

  // Draw QR code (right side)
  drawQrCode(qrContent.c_str(), DisplayManager::getDisplay().width() - 66 - 5,
             (DisplayManager::getDisplay().height() - 66) / 2, 2);

  // Draw text info (left side)
  DisplayManager::getDisplay().setFont(&FreeSans9pt7b);
  DisplayManager::displayText("Pool Monitor", 14, GxEPD_ALIGN_LEFT, 3);
  DisplayManager::getDisplay().drawLine(3, 20, DisplayManager::getDisplay().width() - 70, 20, GxEPD_BLACK);

  DisplayManager::getDisplay().setFont(&FreeSans9pt7b);
  DisplayManager::displayText("Connect to WiFi:", 36, GxEPD_ALIGN_LEFT, 3);

  DisplayManager::getDisplay().setFont(&FreeSansBold9pt7b);
  DisplayManager::displayText(DEVICE_NAME, 52, GxEPD_ALIGN_LEFT, 3);

  DisplayManager::getDisplay().setFont(&FreeSans9pt7b);
  String ipLine = "IP: " + apIP;
  DisplayManager::displayText(ipLine.c_str(), 70, GxEPD_ALIGN_LEFT, 3);

  DisplayManager::getDisplay().setFont(&FreeMono9pt7b);
  DisplayManager::displayText("Scan QR to connect", 88, GxEPD_ALIGN_LEFT, 3);

  DisplayManager::getDisplay().update();
}

void PoolMonitorContext::showWiFiConnectionFailedScreen() {
  Serial.println("🛑\tWiFi connection failed");

  DisplayManager::getDisplay().fillScreen(GxEPD_WHITE);
  DisplayManager::getDisplay().setFont(&FreeSans9pt7b);
  DisplayManager::displayText("Pool Monitor", 18, GxEPD_ALIGN_LEFT);
  DisplayManager::displayText("*** Error ***", 60, GxEPD_ALIGN_CENTER);
  DisplayManager::displayText("WiFi connection failed", 90, GxEPD_ALIGN_LEFT);
  DisplayManager::getDisplay().update();

  // Remove all preferences under the opened namespace
  preferences_->clear();
  preferences_->end();

  ESP.restart();
}

void PoolMonitorContext::showWiFiConnectedScreen() {
  WiFi.waitForConnectResult();

  IPAddress wIP = WiFi.localIP();
  Serial.printf("WiFi IP address: %u.%u.%u.%u\n", wIP[0], wIP[1], wIP[2], wIP[3]);

  // Advertise via mDNS
  if (MDNS.begin(DEVICE_NAME)) {
    Serial.printf("📡\tmDNS responder started: %s.local\n", DEVICE_NAME);
  }

  Serial.printf("Connecting to %s\n", PoolMonitorContext::mqtt_server.c_str());

  // Connect to MQTT
  if (NetworkManager::beginMqtt(PoolMonitorContext::mqtt_server.c_str(),
                                PoolMonitorContext::mqtt_server_port, DEVICE_NAME)) {
    Serial.println("MQTT connected");
  }
}

auto PoolMonitorContext::initializeNetwork() -> void {
  // Configure WiFiSettings
  WiFiSettings.hostname = DEVICE_NAME;

  // Set up callbacks for WiFiSettings
  WiFiSettings.onPortal = []() {
    Serial.println("⚙️\tConfiguration portal active");
    // Show setup screen on display
    showSetupScreen();
  };

  WiFiSettings.onSuccess = []() {
    Serial.println("✅\tWiFi connected successfully");
    showWiFiConnectedScreen();
  };

  WiFiSettings.onFailure = []() {
    Serial.println("🛑\tWiFi connection failed");
    showWiFiConnectionFailedScreen();
  };

  WiFiSettings.onConfigSaved = []() {
    Serial.println("💾\tConfiguration saved - restarting");
    // Ensure first wake after restart does a network cycle immediately
    preferences_->putUInt("no_wifi_count", SKIP_WIFI_WAKE_CYCLES);
    preferences_->end();
    ESP.restart();
  };

  // Define custom settings for MQTT
  PoolMonitorContext::mqtt_server = WiFiSettings.string("mqtt_server", "", "MQTT Hostname");
  PoolMonitorContext::mqtt_server_port = WiFiSettings.integer("mqtt_port", 1, 65535, 1883, "MQTT Port");

  // Save MQTT settings to preferences for later use
  preferences_->putString("mqtt_server", PoolMonitorContext::mqtt_server);
  preferences_->putUInt("mqtt_port", PoolMonitorContext::mqtt_server_port);

  // Connect to WiFi with timeout
  WiFiSettings.connect(true, 45);

  if (!NetworkManager::isWiFiConnected()) {
    Serial.println("🛑\tWiFi connection failed - starting configuration portal");
    // Start the configuration portal
    WiFiSettings.portal();
  }
}

auto PoolMonitorContext::initializeMqtt() -> void {
  // MQTT settings are already loaded from WiFiSettings in initializeNetwork
  // and saved to preferences, so we can use the static variables

  if (PoolMonitorContext::mqtt_server.length() == 0) {
    Serial.println("⚠️\tNo MQTT server configured");
    return;
  }

  // Connect to MQTT
  if (!NetworkManager::beginMqtt(PoolMonitorContext::mqtt_server.c_str(),
                                PoolMonitorContext::mqtt_server_port, DEVICE_NAME)) {
    Serial.println("⚠️\tMQTT connection failed");
    return;
  }

  // Set MQTT callback
  NetworkManager::setMqttCallback(handleMqttMessage);

  // Subscribe to topics
  const char* subscriptions[] = {
    HA_TOPIC_POOL_TEMP,
    HA_TOPIC_SOLAR_TEMP,
    HA_TOPIC_POOL_PUMP,
    HA_TOPIC_SOLAR_PUMP,
    HA_TOPIC_MODE
  };

  size_t successfulSubscriptions = 0;
  for (size_t i = 0; i < sizeof(subscriptions) / sizeof(subscriptions[0]); i++) {
    if (!NetworkManager::subscribe(subscriptions[i])) {
      Serial.printf("🛑\tFailed to subscribe to: %s\n", subscriptions[i]);
    } else {
      successfulSubscriptions++;
    }
  }

  Serial.printf("📡\tMQTT subscriptions successful: %u/%u\n",
                static_cast<unsigned int>(successfulSubscriptions),
                static_cast<unsigned int>(sizeof(subscriptions) / sizeof(subscriptions[0])));

  // Process retained messages (reduced from original 200 to save power)
  for (int i = 0; i < 100; i++) {
    NetworkManager::loop();
    delay(10);
  }
}

auto PoolMonitorContext::loadState() -> void {
  poolTemp_ = preferences_->getFloat("pool_temp", 0.0f);
  solarTemp_ = preferences_->getFloat("solar_temp", 0.0f);
  poolPumpOn_ = preferences_->getBool("pump_pool", false);
  solarPumpOn_ = preferences_->getBool("pump_solar", false);
  poolMode_ = preferences_->getString("pool_mode", "unknown");
  lastUpdate_ = preferences_->getString("last_update", "??:??");

  Serial.printf("📥\tLoaded state: Pool=%.1f°C, Solar=%.1f°C, PoolPump=%s, SolarPump=%s, Mode=%s\n",
                poolTemp_, solarTemp_, poolPumpOn_ ? "ON" : "OFF", solarPumpOn_ ? "ON" : "OFF",
                poolMode_.c_str());

  stateLoaded_ = true;
}

auto PoolMonitorContext::saveState() -> void {
  preferences_->putFloat("pool_temp", poolTemp_);
  preferences_->putFloat("solar_temp", solarTemp_);
  preferences_->putBool("pump_pool", poolPumpOn_);
  preferences_->putBool("pump_solar", solarPumpOn_);
  preferences_->putString("pool_mode", poolMode_);
  preferences_->putString("last_update", lastUpdate_);

  Serial.println("💾\tState saved");
}

// Case-insensitive ASCII string comparison
static bool equalsIgnoreCaseAscii(const char* lhs, const char* rhs) {
  if (lhs == nullptr || rhs == nullptr) {
    return false;
  }
  while (*lhs != '\0' && *rhs != '\0') {
    if (std::tolower(static_cast<unsigned char>(*lhs)) !=
        std::tolower(static_cast<unsigned char>(*rhs))) {
      return false;
    }
    lhs++;
    rhs++;
  }
  return *lhs == *rhs;
}

// Parse boolean MQTT state payloads used by Home Assistant topics
static bool parseHomeAssistantBoolState(const char* value) {
  if (value == nullptr) {
    return false;
  }
  if (equalsIgnoreCaseAscii(value, "true")
      || equalsIgnoreCaseAscii(value, "on")
      || equalsIgnoreCaseAscii(value, "1")) {
    return true;
  }
  if (equalsIgnoreCaseAscii(value, "false")
      || equalsIgnoreCaseAscii(value, "off")
      || equalsIgnoreCaseAscii(value, "0")) {
    return false;
  }
  Serial.printf("⚠️\tUnexpected boolean MQTT payload: %s (defaulting to false)\n", value);
  return false;
}

void PoolMonitorContext::handleMqttMessage(char* topic, byte* payload, unsigned int length) {
  // Stack-allocated buffer instead of heap allocation
  char payloadCopy[MQTT_PAYLOAD_BUFFER_SIZE];
  size_t payloadLength = length;
  if (payloadLength >= sizeof(payloadCopy)) {
    payloadLength = sizeof(payloadCopy) - 1;
    Serial.printf("⚠️\tMQTT payload truncated for topic %s (original: %u bytes, buffer: %zu bytes)\n",
                  topic, length, sizeof(payloadCopy));
  }
  memcpy(payloadCopy, payload, payloadLength);
  payloadCopy[payloadLength] = '\0';

  String payloadString = String(payloadCopy);

  // Match Home Assistant state topics directly
  if (strcmp(topic, HA_TOPIC_POOL_TEMP) == 0) {
    Serial.println("\tPool temperature: " + payloadString);
    poolTemp_ = payloadString.toFloat();
    preferences_->putFloat("pool_temp", poolTemp_);

  } else if (strcmp(topic, HA_TOPIC_SOLAR_TEMP) == 0) {
    Serial.println("\tSolar temperature: " + payloadString);
    solarTemp_ = payloadString.toFloat();
    preferences_->putFloat("solar_temp", solarTemp_);

  } else if (strcmp(topic, HA_TOPIC_POOL_PUMP) == 0) {
    Serial.println("\tPool pump: " + payloadString);
    poolPumpOn_ = parseHomeAssistantBoolState(payloadCopy);
    preferences_->putBool("pump_pool", poolPumpOn_);

  } else if (strcmp(topic, HA_TOPIC_SOLAR_PUMP) == 0) {
    Serial.println("\tSolar pump: " + payloadString);
    solarPumpOn_ = parseHomeAssistantBoolState(payloadCopy);
    preferences_->putBool("pump_solar", solarPumpOn_);

  } else if (strcmp(topic, HA_TOPIC_MODE) == 0) {
    Serial.println("\tOperation Mode: " + payloadString);
    poolMode_ = payloadString;
    preferences_->putString("pool_mode", poolMode_);
  }
}

auto PoolMonitorContext::isMqttConnected() -> bool {
  return NetworkManager::isMqttConnected();
}

auto PoolMonitorContext::isWiFiConnected() -> bool {
  return NetworkManager::isWiFiConnected();
}

bool PoolMonitorContext::isNtpSyncNeeded() {
  unsigned long last_ntp_sync = preferences_->getULong("last_ntp_sync", 0);
  unsigned long total_uptime = preferences_->getULong("total_uptime", 0);

  // If never synced or more than NTP_SYNC_INTERVAL_SECONDS have passed
  if (last_ntp_sync == 0 || (total_uptime - last_ntp_sync) >= NTP_SYNC_INTERVAL_SECONDS) {
    return true;
  }
  return false;
}

}  // namespace PoolMonitor

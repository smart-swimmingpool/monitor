// Copyright 2020 Smart Swimmingpool
/**
  Monitor to show temperature of smart-swimmingpool:

  ESP8266
   - ESP8266 NodeMCU Contoller
   - LiquidCrystal I2C 16*2 Display

  TTGO T5 E-Paper
   - ESP32 Contoller
   - 2.13inch e-Paper
*/

#include <Arduino.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <qrcode.h>
#include <cctype>


#define LILYGO_T5_V213 1  // see defines in board_def.h

#include "board_def.h"
#include "ntp_localtime.h"
#include "u8g2_display.h"

#define MODEM_POWER_ON 23
#define LED_BUILTIN 2  // built-in LED on TTGO-T5

const char*    DEVICE_NAME           = "pool-monitor";
const int32_t  TIME_TO_SLEEP_SECONDS = 180;   // Time ESP32 will go to sleep (in seconds)
const int32_t  NTP_SYNC_INTERVAL_SECONDS = 3600;  // Sync NTP time every hour (3600 seconds)

// MQTT settings
String mqtt_server;
u_int16_t mqtt_server_port;
IPAddress  remote;     // IP Address of mqtt server

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds

// Home Assistant MQTT state topics for pool-controller (fixed, no discovery needed)
const char* HA_TOPIC_POOL_TEMP   = "homeassistant/sensor/pool-controller/pool-temp/state";
const char* HA_TOPIC_SOLAR_TEMP  = "homeassistant/sensor/pool-controller/solar-temp/state";
const char* HA_TOPIC_POOL_PUMP   = "homeassistant/switch/pool-controller/pool-pump/state";
const char* HA_TOPIC_SOLAR_PUMP  = "homeassistant/switch/pool-controller/solar-pump/state";
const char* HA_TOPIC_MODE        = "homeassistant/select/pool-controller/mode/state";

// Buffer size for MQTT payload copy from callback.
// 128 bytes cover expected short state payloads (temperature, ON/OFF, mode).
const size_t MQTT_PAYLOAD_BUFFER_SIZE = 128;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


// data stored during deep sleep
Preferences preferences;

/**
 * @brief Check if NTP time sync is needed based on last sync timestamp
 * @return true if sync is needed, false otherwise
 */
bool isNtpSyncNeeded() {
  unsigned long last_ntp_sync = preferences.getULong("last_ntp_sync", 0);
  unsigned long time_since_boot = preferences.getULong("total_uptime", 0);

  // If never synced or more than NTP_SYNC_INTERVAL_SECONDS have passed
  if (last_ntp_sync == 0 || (time_since_boot - last_ntp_sync) >= NTP_SYNC_INTERVAL_SECONDS) {
    return true;
  }
  return false;
}

/**
 * @brief Initialize the e-ink display with static content, icons, and layout lines.
 *
 */
void initDisplay() {
  Serial.println("🖥️\tInitializing display...");

  display.fillScreen(GxEPD_WHITE);
  display.setRotation(3);
  display.setTextColor(GxEPD_BLACK);

  // draw icons
  u8g2_for_adafruit_gfx.setFontMode(0);
  u8g2_for_adafruit_gfx.setForegroundColor(0);
  u8g2_for_adafruit_gfx.setBackgroundColor(1);

  u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_all_t);
  u8g2_for_adafruit_gfx.drawGlyph(4, 50, 0x02a6); /* hex pool */
  display.setFont(&FreeSans12pt7b);
  displayText("     Pool:", 50, GxEPD_ALIGN_LEFT);

  u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_ecology_t);
  u8g2_for_adafruit_gfx.drawGlyph(4, 94, 0x003E); /* hex 3E solar panel */
  display.setFont(&FreeSans12pt7b);
  displayText("     Solar:", 94, GxEPD_ALIGN_LEFT);

  display.drawLine(0, 102, display.width(), 102, GxEPD_BLACK);
  display.drawLine(0, 103, display.width(), 103, GxEPD_BLACK);
  display.setFont(&FreeSans9pt7b);
  displayText("www.smart-swimmingpool.com", 117, GxEPD_ALIGN_LEFT);

  display.update();
}

void updateDisplay() {
  Serial.println("🖥️\tUpdating display");

  const int16_t UPDATE_AREA_X = 90;
  const int16_t UPDATE_AREA_Y = 0;
  const int16_t UPDATE_AREA_WIDTH = display.width() - UPDATE_AREA_X;
  const int16_t UPDATE_AREA_HEIGHT = 95;

  // Buffer for temperature display strings (max: "XX.X C" = 6 chars + null terminator)
  char buffer[50];

  display.fillRect(UPDATE_AREA_X, UPDATE_AREA_Y, UPDATE_AREA_WIDTH, UPDATE_AREA_HEIGHT, GxEPD_WHITE);

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMono9pt7b);
  displayText(preferences.getString("pool_mode", "unknown").c_str(), 10, GxEPD_ALIGN_CENTER);

  display.setFont(&FreeMono9pt7b);
  displayText(preferences.getString("last_update", "??:??").c_str(), 10, GxEPD_ALIGN_RIGHT);

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold24pt7b);
  // Use snprintf for safer buffer handling
  snprintf(buffer, sizeof(buffer), "%2.1f C", preferences.getFloat("pool_temp", 0.0));
  displayText(buffer, 50, GxEPD_ALIGN_RIGHT);
  display.drawCircle(display.width() - 35, 20, 5, GxEPD_BLACK);
  display.drawCircle(display.width() - 35, 20, 4, GxEPD_BLACK);
  display.drawCircle(display.width() - 35, 20, 3, GxEPD_BLACK);

  if (preferences.getBool("pump_pool", false)) {
    u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_all_t);
    u8g2_for_adafruit_gfx.drawGlyph(95, 48, 0x01ec); /* run circle */
  } else {
    display.fillRect(95, 48 - 5, 8, 8, GxEPD_WHITE);
  }

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold24pt7b);
  // Use snprintf for safer buffer handling
  snprintf(buffer, sizeof(buffer), "%2.1f C", preferences.getFloat("solar_temp", 0.0));
  displayText(buffer, 94, GxEPD_ALIGN_RIGHT);
  display.drawCircle(display.width() - 35, 64, 5, GxEPD_BLACK);
  display.drawCircle(display.width() - 35, 64, 4, GxEPD_BLACK);
  display.drawCircle(display.width() - 35, 64, 3, GxEPD_BLACK);

  if (preferences.getBool("pump_solar", false)) {
    u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_all_t);
    u8g2_for_adafruit_gfx.drawGlyph(95, 94, 0x01ec); /* run circle */
  } else {
    display.fillRect(95, 94 - 5, 8, 8, GxEPD_WHITE);
  }

  display.updateWindow(UPDATE_AREA_X, UPDATE_AREA_Y, UPDATE_AREA_WIDTH - 1, UPDATE_AREA_HEIGHT - 1, true);
  // display.update();
  delay(5 * 1000);
  // display.powerDown();
}


/**
 * Case-insensitive ASCII string comparison.
 */
static bool equalsIgnoreCaseAscii(const char* lhs, const char* rhs) {
  if (lhs == nullptr || rhs == nullptr) {
    return false;
  }
  while (*lhs != '\0' && *rhs != '\0') {
    if (std::tolower(static_cast<unsigned char>(*lhs)) != std::tolower(static_cast<unsigned char>(*rhs))) {
      return false;
    }
    lhs++;
    rhs++;
  }
  return *lhs == *rhs;
}

/**
 * Parse boolean MQTT state payloads used by Home Assistant topics.
 * Accepted truthy values: "true", "on", "1" (case-insensitive).
 */
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

/**
 *  @brief called on MQTT message
 */
void onMqttCallback(char* topic, byte* payload, unsigned int length) {
  // Stack-allocated buffer instead of heap allocation (Deep-Sleep-sicher, kein Fragmentierungsrisiko)
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
    preferences.putFloat("pool_temp", payloadString.toFloat());

  } else if (strcmp(topic, HA_TOPIC_SOLAR_TEMP) == 0) {
    Serial.println("\tSolar temperature: " + payloadString);
    preferences.putFloat("solar_temp", payloadString.toFloat());

  } else if (strcmp(topic, HA_TOPIC_POOL_PUMP) == 0) {
    Serial.println("\tPool pump: " + payloadString);
    preferences.putBool("pump_pool", parseHomeAssistantBoolState(payloadCopy));

  } else if (strcmp(topic, HA_TOPIC_SOLAR_PUMP) == 0) {
    Serial.println("\tSolar pump: " + payloadString);
    preferences.putBool("pump_solar", parseHomeAssistantBoolState(payloadCopy));

  } else if (strcmp(topic, HA_TOPIC_MODE) == 0) {
    Serial.println("\tOperation Mode: " + payloadString);
    preferences.putString("pool_mode", payloadString);
  } else {
    // Serial.println("Unmanaged mqtt message: " + String(topic));
  }
}

/**
 * Connect MQTT Server
 */
void connectMQTT(IPAddress ip) {
  Serial.printf("Attempting MQTT connection to %s ...\n", mqtt_server.c_str());
  mqttClient.setServer(mqtt_server.c_str(), mqtt_server_port);
  mqttClient.setCallback(onMqttCallback);

  // Attempt to connect
  if (mqttClient.connect(DEVICE_NAME)) {
    Serial.println(F("MQTT connected."));

    // Subscribe to Home Assistant state topics with per-subscription diagnostics
    bool subscriptionFailed = false;
    size_t successfulSubscriptions = 0;
    const char* subscriptions[] = {
      HA_TOPIC_POOL_TEMP,
      HA_TOPIC_SOLAR_TEMP,
      HA_TOPIC_POOL_PUMP,
      HA_TOPIC_SOLAR_PUMP,
      HA_TOPIC_MODE
    };
    for (size_t i = 0; i < sizeof(subscriptions) / sizeof(subscriptions[0]); i++) {
      if (!mqttClient.subscribe(subscriptions[i])) {
        Serial.printf("🛑\tFailed to subscribe to: %s\n", subscriptions[i]);
        subscriptionFailed = true;
      } else {
        successfulSubscriptions++;
      }
    }
    Serial.printf("📡\tMQTT subscriptions successful: %u/%u\n",
                  static_cast<unsigned int>(successfulSubscriptions),
                  static_cast<unsigned int>(sizeof(subscriptions) / sizeof(subscriptions[0])));
    if (subscriptionFailed) {
      Serial.println("⚠️\tRunning in degraded mode due to missing MQTT subscriptions");
    }

  } else {
    Serial.printf("failed, rc=%d\n", mqttClient.state());
    // Print to know why the connection failed
    // See http://pubsubclient.knolleary.net/api.html#state for the failure code and its reason
    switch (mqttClient.state()) {
      case -4:
        Serial.println(F("MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time"));
        break;
      case -3:
        Serial.println(F("MQTT_CONNECTION_LOST - the network connection was broken"));
        break;
      case -2:
        Serial.println(F("MQTT_CONNECT_FAILED - the network connection failed"));
        break;
      case -1:
        Serial.println(F("MQTT_DISCONNECTED - the client is disconnected cleanly"));
        break;
    }
  }
}


// Context for QR code display callback (ESP32 framework passes no user data)
static struct {
  int16_t x, y;
  uint8_t scale;
} _qr_ctx;

/**
 * Display callback for esp_qrcode_generate — draws modules on the E-Ink display.
 */
static void _qrDisplayFunc(esp_qrcode_handle_t qrcode) {
  int size  = esp_qrcode_get_size(qrcode);
  int quiet = 4 * _qr_ctx.scale;

  // Quiet zone (white border)
  display.fillRect(_qr_ctx.x, _qr_ctx.y,
                   size * _qr_ctx.scale + quiet * 2,
                   size * _qr_ctx.scale + quiet * 2,
                   GxEPD_WHITE);

  // Draw modules as black squares
  for (int qy = 0; qy < size; qy++) {
    for (int qx = 0; qx < size; qx++) {
      if (esp_qrcode_get_module(qrcode, qx, qy)) {
        display.fillRect(_qr_ctx.x + quiet + qx * _qr_ctx.scale,
                         _qr_ctx.y + quiet + qy * _qr_ctx.scale,
                         _qr_ctx.scale, _qr_ctx.scale, GxEPD_BLACK);
      }
    }
  }
}

/**
 * Draw a QR code on the display at the given position.
 *
 * Uses the ESP32 framework's built-in QR code library (esp_qrcode).
 *
 * @param text  Text/URL to encode (e.g. "WIFI:T:nopass;S:pool-monitor;;")
 * @param x     Top-left x position
 * @param y     Top-left y position
 * @param scale Pixel size per QR module (2 = well-scannable on 2.13" E-Ink)
 */
static void drawQrCode(const char* text, int16_t x, int16_t y, uint8_t scale) {
  _qr_ctx.x = x;
  _qr_ctx.y = y;
  _qr_ctx.scale = scale;

  esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
  cfg.display_func       = _qrDisplayFunc;
  cfg.max_qrcode_version = 3;   // enough for ~55 bytes of alphanumeric data
  cfg.qrcode_ecc_level   = ESP_QRCODE_ECC_LOW;

  if (esp_qrcode_generate(&cfg, text) != ESP_OK) {
    Serial.println("⚠️\tQR code generation failed");
  }
}

void showSetupScreen() {
  Serial.println("⚙️\tConfiguration portal active");

  // Build the WiFi QR code string
  // Format: WIFI:T:<security>;S:<ssid>;P:<password>;;
  String qrContent = "WIFI:T:nopass;S:" + String(DEVICE_NAME) + ";;";

  String apIP = WiFi.softAPIP().toString();

  display.fillScreen(GxEPD_WHITE);
  display.setRotation(3);

  // --- QR code (right side) ---
  // Version 2 at scale=2: 25 modules × 2 + 8 quiet = 66×66 px
  drawQrCode(qrContent.c_str(), display.width() - 66 - 5, (display.height() - 66) / 2, 2);

  // --- Text info (left side) ---
  display.setFont(&FreeSans9pt7b);
  displayText("Pool Monitor", 14, GxEPD_ALIGN_LEFT, 3);
  display.drawLine(3, 20, display.width() - 70, 20, GxEPD_BLACK);

  display.setFont(&FreeSans9pt7b);
  displayText("Connect to WiFi:", 36, GxEPD_ALIGN_LEFT, 3);

  display.setFont(&FreeSansBold9pt7b);
  displayText(DEVICE_NAME, 52, GxEPD_ALIGN_LEFT, 3);

  display.setFont(&FreeSans9pt7b);
  String ipLine = "IP: " + apIP;
  displayText(ipLine.c_str(), 70, GxEPD_ALIGN_LEFT, 3);

  display.setFont(&FreeMono9pt7b);
  displayText("Scan QR to connect", 88, GxEPD_ALIGN_LEFT, 3);

  display.update();

  // Reset all preferences (clear keys, but keep namespace open).
  // Do NOT call preferences.end() here — if the portal times out without saving,
  // setup() continues with preferences.get*/put* calls and would hit defaults
  // or fail silently with a closed namespace.
  preferences.clear();
}

void showWiFiConnectionFailedScreen() {
  Serial.println("🛑\tWiFi connection failed");

  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeSans9pt7b);
  displayText("Pool Monitor", 18, GxEPD_ALIGN_LEFT);
  displayText("*** Error ***", 60, GxEPD_ALIGN_CENTER);
  displayText("WiFi connection failed", 90, GxEPD_ALIGN_LEFT);
  display.update();

  // Remove all preferences under the opened namespace
  preferences.clear();

  // Close preferences before restart to prevent corruption
  preferences.end();

  ESP.restart();
}

void showWiFiConnectedScreen() {
  WiFi.waitForConnectResult();

  IPAddress wIP = WiFi.localIP();
  Serial.printf("WiFi IP address: %u.%u.%u.%u\n", wIP[0], wIP[1], wIP[2], wIP[3]);

  // Advertise via mDNS so the device is discoverable as pool-monitor.local
  if (MDNS.begin(DEVICE_NAME)) {
    Serial.printf("📡\tmDNS responder started: %s.local\n", DEVICE_NAME);
  }

  Serial.printf("Connecting to %s\n", mqtt_server.c_str());
  WiFi.hostByName(mqtt_server.c_str(), remote);

  if (remote != INADDR_NONE) {
    Serial.printf("Connecting to mqtt server: %s (IP: %u.%u.%u.%u)\n", mqtt_server.c_str(), remote[0], remote[1],
                  remote[2], remote[3]);
  } else {
    Serial.printf("Could not resolve hostname: %s, connecting via hostname string\n", mqtt_server.c_str());
  }

  // Always attempt MQTT connection — connectMQTT() uses the hostname string internally
  // and PubSubClient resolves DNS on its own, so a pre-resolution failure is not fatal.
  connectMQTT(remote);
  if (mqttClient.connected()) {
    updateDisplay();
  }
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      initDisplay();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println(F(" ------------------------------------- "));
  Serial.println(F("| Pool Monitor                        |"));
  Serial.println(F("| www.smart-swimmingpool.com          |"));
  Serial.println(F(" ------------------------------------- "));

  // initialize the display
  display.init();
  u8g2_for_adafruit_gfx.begin(display);
  display.fillScreen(GxEPD_WHITE);
  display.setRotation(3);

  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
  if (!preferences.begin("pool-monitor", false)) {
    Serial.println("🛑\tFailed to open preferences");
    while (1) {
      delay(1000);
    }
  }
  // Remove all preferences under the opened namespace
  // preferences.clear();
  Serial.printf("Number of free entries in prefs: %d\n", preferences.freeEntries());

  unsigned int boot_count = preferences.getUInt("boot_count", 0);
  // Increment boot number and print it every reboot
  Serial.printf("Current boot count: %u\n", ++boot_count);
  // Store the counter to the Preferences
  preferences.putUInt("boot_count", boot_count);

  // Track cumulative uptime across sleep cycles for NTP sync scheduling.
  // Increment by sleep duration to track total time across deep sleep cycles.
  unsigned long total_uptime = preferences.getULong("total_uptime", 0);
  total_uptime += TIME_TO_SLEEP_SECONDS;
  preferences.putULong("total_uptime", total_uptime);
  Serial.printf("Total uptime: %lu seconds (%.1f hours)\n", total_uptime, total_uptime / 3600.0);

  // Print the wakeup reason for ESP32
  print_wakeup_reason();

  SPIFFS.begin(true);  // Will format on the first run after failing to mount

  WiFiSettings.hostname       = DEVICE_NAME;
  WiFiSettings.onPortal       = []() { showSetupScreen(); };
  WiFiSettings.onSuccess      = []() { showWiFiConnectedScreen(); };
  WiFiSettings.onFailure      = []() { showWiFiConnectionFailedScreen(); };
  WiFiSettings.onConfigSaved  = []() {
    preferences.end();  // Close preferences before restart
    ESP.restart();
  };  // Reboot as soon as config is saved

  // Define custom settings saved by WifiSettings
  // These will return the default if nothing was set before
  mqtt_server     = WiFiSettings.string("mqtt_server", "hostname", "MQTT Hostname");
  mqtt_server_port = WiFiSettings.integer("mqtt_port", 1, 65535, 1883, "MQTT Port");

  // Connect to WiFi with a timeout of 45 seconds
  // Launches the portal if the connection failed
  WiFiSettings.connect(true, 45);

  // If MQTT is not reachable, stay awake and open the configuration portal
  // so the user can fix settings (MQTT hostname, port) instead of cycling
  // through sleep/wake/fail loops.
  if (!mqttClient.connected()) {
    Serial.println("🛑\tMQTT server not reachable - starting configuration portal");
    Serial.println("📡\tConnect to the '" + String(DEVICE_NAME) + "' access point to configure MQTT settings");

    // Show MQTT error briefly — showSetupScreen() (called by portal)
    // will overwrite this with the full AP info + QR code.
    display.fillScreen(GxEPD_WHITE);
    display.setRotation(3);
    display.setFont(&FreeSans9pt7b);
    displayText("Pool Monitor", 14, GxEPD_ALIGN_LEFT, 3);
    display.drawLine(3, 20, display.width() - 3, 20, GxEPD_BLACK);
    displayText("*** MQTT Error ***", 55, GxEPD_ALIGN_CENTER);
    displayText("Starting config...", 80, GxEPD_ALIGN_CENTER);
    display.update();

    // Start the captive portal for reconfiguration.
    // This loops indefinitely until the user saves new settings,
    // which triggers the onConfigSaved callback → ESP.restart().
    WiFiSettings.portal();
    // Not reached (portal loops forever or restarts)
  }

  // Initialize NTP client and sync time only when needed (reduces network traffic and power consumption)
  timeClient.begin();

  if (isNtpSyncNeeded()) {
    Serial.println("⏰\tNTP sync needed - updating time from server");
    String currentTime = getCurrentTime();
    preferences.putString("last_update", currentTime);

    // Update last sync timestamp
    preferences.putULong("last_ntp_sync", total_uptime);

    // Store the epoch time at the moment of successful NTP sync
    unsigned long synced_epoch = timeClient.getEpochTime();
    preferences.putULong("last_epoch", synced_epoch);

    Serial.printf("⏰\tNTP synced successfully at %s (next sync in ~%d seconds)\n",
                  currentTime.c_str(), NTP_SYNC_INTERVAL_SECONDS);
  } else {
    Serial.println("⏰\tNTP sync skipped - using cached time from stored epoch and uptime");

    // Reconstruct current time based on last known epoch and elapsed uptime since last sync
    unsigned long last_epoch     = preferences.getULong("last_epoch", 0);
    unsigned long last_ntp_sync  = preferences.getULong("last_ntp_sync", 0);

    // Guard against underflow if stored values are inconsistent
    unsigned long elapsed_since_sync = 0;
    if (total_uptime > last_ntp_sync) {
      elapsed_since_sync = total_uptime - last_ntp_sync;
    }

    time_t t = currentTZ.toLocal(last_epoch + elapsed_since_sync);
    char buf[10];
    snprintf(buf, sizeof(buf), "%.2d:%.2d", hour(t), minute(t));
    preferences.putString("last_update", String(buf));
  }

  // Process MQTT messages with timeout (2 seconds max, 200 * 10ms)
  // Retained messages arrive immediately after subscribe, so this is sufficient
  for (int i = 0; i < 200; i++) {
    mqttClient.loop();  // Ensure we've sent & received everything
    delay(10);
  }

  Serial.printf("😴\tGoing to sleep now for %d sec.\n", (TIME_TO_SLEEP_SECONDS));

  // Properly disconnect MQTT client to prevent memory leak
  if (mqttClient.connected()) {
    mqttClient.disconnect();
  }

  // NOTE: No explicit WiFi disconnection here — Deep Sleep powers down the radio
  // automatically. An explicit disconnect would send DHCPRELEASE to the router,
  // removing the device from the DHCP client table and making it invisible on
  // the network between wake cycles. Letting Deep Sleep handle the power-down
  // preserves the DHCP lease so the device stays visible (though unresponsive
  // during sleep) and reuses the same IP on wake.

  updateDisplay();
  delay(3000);  // Wait for the display to update
  display.powerDown();

  // Close SPIFFS to prevent flash handle leak over sleep cycles
  SPIFFS.end();

  // Close the Preferences
  preferences.end();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_SECONDS * uS_TO_S_FACTOR);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_POWER_ON, LOW);
  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  pinMode(LED_BUILTIN, OUTPUT);

  esp_deep_sleep_start();
  Serial.println("🛌🏼 This will never be printed");
}

/**
 * @brief loop not used, we go to sleep.
 *
 */
void loop() {}

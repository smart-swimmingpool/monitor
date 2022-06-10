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
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <Preferences.h>


#define LILYGO_T5_V213 1  // see defines in board_def.h

#include "board_def.h"
#include "ntp_localtime.h"
#include "u8g2_display.h"

#define MODEM_POWER_ON 23
#define LED_BUILTIN 2  // built-in LED on TTGO-T5

const char*    DEVICE_NAME           = "pool-monitor";
const int32_t  TIME_TO_SLEEP_SECONDS = 180;   // Time ESP32 will go to sleep (in seconds)

//MQTT settings
String mqtt_server;
u_int16_t mqtt_server_port;
IPAddress  remote;     // IP Address of mqtt server

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


// data stored during deep sleep
Preferences preferences;

/**
 * @brief
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
  u8g2_for_adafruit_gfx.drawGlyph(4, 45, 0x02a6); /* hex pool */
  display.setFont(&FreeSans12pt7b);
  displayText("     Pool:", 45, GxEPD_ALIGN_LEFT);

  u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_ecology_t);
  u8g2_for_adafruit_gfx.drawGlyph(4, 90, 0x003E); /* hex 3E solar panel */
  display.setFont(&FreeSans12pt7b);
  displayText("     Solar:", 90, GxEPD_ALIGN_LEFT);

  display.drawLine(0, 100, display.width(), 100, GxEPD_BLACK);
  display.drawLine(0, 101, display.width(), 101, GxEPD_BLACK);
  display.setFont(&FreeSans9pt7b);
  displayText("www.smart-swimmingpool.com", 117, GxEPD_ALIGN_LEFT);

  display.update();
}

void updateDisplay() {
  Serial.println("🖥️\tUpdating display");

  const int16_t UPDATE_AREA_X = 90;
  const int16_t UPDATE_AREA_Y = 0;
  const int16_t UPDATE_AREA_WIDTH = display.width() - UPDATE_AREA_X;
  const int16_t UPDATE_AREA_HEIGHT = 99;

  char buffer[50];

  display.fillRect(UPDATE_AREA_X, UPDATE_AREA_Y, UPDATE_AREA_WIDTH, UPDATE_AREA_HEIGHT, GxEPD_WHITE);

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMono9pt7b);
  displayText(preferences.getString("pool_mode", "unknown").c_str(), 10, GxEPD_ALIGN_CENTER);

  display.setFont(&FreeMono9pt7b);
  displayText(preferences.getString("last_update", "??:??").c_str(), 10, GxEPD_ALIGN_RIGHT);

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold24pt7b);

  sprintf(buffer, "%2.1f\xB0"" C", preferences.getFloat("pool_temp", 0.0));
  displayText(buffer, 48, GxEPD_ALIGN_RIGHT);
  display.drawCircle(display.width() - 35, 18, 4, GxEPD_BLACK);

  if(preferences.getBool("pump_pool", false)) {
    u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_all_t);
    u8g2_for_adafruit_gfx.drawGlyph(95 , 48, 0x01ec); /* run circle */
  } else {
    display.fillRect(95, 48 -5, 8, 8, GxEPD_WHITE);
  }

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold24pt7b);
  sprintf(buffer, "%2.1f\xB0"" C", preferences.getFloat("solar_temp", 0.0));
  displayText(buffer, 90, GxEPD_ALIGN_RIGHT);
  display.drawCircle(display.width() - 35, 60, 4, GxEPD_BLACK);

  if(preferences.getBool("pump_solar", false)) {
    u8g2_for_adafruit_gfx.setFont(u8g2_font_streamline_all_t);
    u8g2_for_adafruit_gfx.drawGlyph(95 , 90, 0x01ec); /* run circle */
  } else {
    display.fillRect(95, 90 -5, 8, 8, GxEPD_WHITE);
  }

  display.updateWindow(UPDATE_AREA_X, UPDATE_AREA_Y, UPDATE_AREA_WIDTH - 1, UPDATE_AREA_HEIGHT - 1, true);
  // display.update();
  delay(5 * 1000);
  //display.powerDown();
}


/**
 *  @brief called on MQTT message
 */
void onMqttCallback(char* topic, byte* payload, unsigned int length) {
  String command = String((char*) topic);
  String payloadString = String((char*) payload).substring(0, length);
  String device_id = preferences.getString("device_id");

  if(command.endsWith("/$name") && device_id.length() == 0) {
    if(payloadString.startsWith("Pool Controller")) {
      // got the name of the device
      device_id = command.substring(/*"homie/"*/ 6, command.length() - 6 /*"/$name"*/);
      Serial.println("🏊🏼\tPool Controller found using id " + device_id);
      preferences.putString("device_id", device_id);

      String pool_controller = "homie/" + device_id + "/#";
      mqttClient.subscribe(pool_controller.c_str());
      Serial.println("🏊🏼\tSubscribed to: " + pool_controller);

      mqttClient.unsubscribe("homie/+/$name"); //no longer required to search.
    }

  } else if(command.endsWith("/pool-temp/temperature")) {

    Serial.println("🏊🏼🌡️  Pool temperature: " + payloadString);
    preferences.putFloat("pool_temp", payloadString.toFloat());

  } else if(command.endsWith("/solar-temp/temperature")) {

    Serial.println("🌞🌡️\tSolar temperature: " + payloadString);
    preferences.putFloat("solar_temp", payloadString.toFloat());

  } else if(command.endsWith("/pool-pump/switch")) {
    Serial.println("⚙️💧\tPool pump: " + payloadString);
    preferences.putBool("pump_pool", (payloadString == "true" ? true : false));

  } else if(command.endsWith("/solar-pump/switch")) {
    Serial.println("⚙️☀️\tSolar pump: " + payloadString);
    preferences.putBool("pump_solar", (payloadString == "true" ? true : false));

  } else if(command.endsWith("/operation-mode/mode")) {
    Serial.println("Operation Mode: " + payloadString);
    preferences.putString("pool_mode", payloadString);
  } else {
    // Serial.println("Unmanaged mqtt message: " + command);
  }

  // updateDisplay();
}

/**
    Connect MQTT Server
*/
void connectMQTT(IPAddress ip) {
  Serial.printf("Attempting MQTT connection to %s ...\n", mqtt_server.c_str());
  mqttClient.setServer(mqtt_server.c_str(), mqtt_server_port);
  mqttClient.setCallback(onMqttCallback);

  // Attempt to connect
  if (mqttClient.connect(DEVICE_NAME)) {

    String device_id = preferences.getString("device_id");

    if(device_id.length() > 0) {
      String pool_controller = "homie/" + device_id + "/#";
      mqttClient.subscribe(pool_controller.c_str());
    } else {
      Serial.println("🏊🏼\tConnected to MQTT server searching device");
      mqttClient.subscribe("homie/+/$name");
    }
    Serial.println(F("MQTT connected."));

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


void showSetupScreen() {
  Serial.println("⚙️\tsetup device");

  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeSans9pt7b);
  displayText("Pool Monitor", 18, GxEPD_ALIGN_LEFT);
  displayText("*** Setup ***", 50, GxEPD_ALIGN_CENTER);
  displayText("Connect to WiFi & add data:", 80, GxEPD_ALIGN_CENTER);
  displayText(DEVICE_NAME, 110, GxEPD_ALIGN_CENTER);
  display.update();

  // Remove all preferences under the opened namespace
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

  ESP.restart();
}

void showWiFiConnectedScreen() {

  WiFi.waitForConnectResult();

  IPAddress wIP = WiFi.localIP();
  Serial.printf("WiFi IP address: %u.%u.%u.%u\n", wIP[0], wIP[1], wIP[2], wIP[3]);

  Serial.printf("Connecting to %s\n", mqtt_server.c_str());
  WiFi.hostByName(mqtt_server.c_str(), remote);

  if (remote != INADDR_NONE) {
    Serial.printf("Connecting to mqtt server: %s (IP: %u.%u.%u.%u)\n", mqtt_server.c_str(), remote[0], remote[1],
                  remote[2], remote[3]);

    connectMQTT(remote);
    updateDisplay();
  } else {
    Serial.printf("Could not resolve hostname: %s\n", mqtt_server.c_str());
  }
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
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
    default :
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason);
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
  };
  // Remove all preferences under the opened namespace
  //preferences.clear();
  Serial.printf("Number of free entries in prefs: %d\n", preferences.freeEntries());

  unsigned int boot_count = preferences.getUInt("boot_count", 0);
  //Increment boot number and print it every reboot
  Serial.printf("Current boot count: %u\n", ++boot_count);
  // Store the counter to the Preferences
  preferences.putUInt("boot_count", boot_count);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  SPIFFS.begin(true);  // Will format on the first run after failing to mount

  WiFiSettings.hostname       = DEVICE_NAME;
  WiFiSettings.onPortal       = []() { showSetupScreen(); };
  WiFiSettings.onSuccess      = []() { showWiFiConnectedScreen(); };
  WiFiSettings.onFailure      = []() { showWiFiConnectionFailedScreen(); };
  WiFiSettings.onConfigSaved  = []() { ESP.restart();  }; // Reboot as soon as config is saved

  // Define custom settings saved by WifiSettings
  // These will return the default if nothing was set before
  mqtt_server     = WiFiSettings.string("mqtt_server", "hostname", "MQTT Hostname");
  mqtt_server_port = WiFiSettings.integer("mqtt_port", 1, 65535, 1883, "MQTT Port");

  // Connect to WiFi with a timeout of 30 seconds
  // Launches the portal if the connection failed
  WiFiSettings.connect(true, 45);

  // Initialize a NTPClient to get time
  timeClient.begin();
  preferences.putString("last_update",  getCurrentTime());

  for(int i=0;i<1000; i++) {
    mqttClient.loop();  //Ensure we've sent & received everything
    //Serial.print(i);
    delay(10);
  }

  Serial.printf("😴 Going to sleep now for %d sec.\n", (TIME_TO_SLEEP_SECONDS));

  WiFi.disconnect(true);  // Disconnect from the network
  delay( 1 );
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
  delay( 1 );

  updateDisplay();
  display.powerDown();

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

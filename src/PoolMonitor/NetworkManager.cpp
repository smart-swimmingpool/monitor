// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file NetworkManager.cpp
 * @brief Implementation of NetworkManager for Pool Monitor.
 */

#include "NetworkManager.hpp"
#include "Config.hpp"
#include <ESPmDNS.h>

namespace PoolMonitor {

// Initialize static members
WiFiClient NetworkManager::wifiClient_;
PubSubClient NetworkManager::mqttClient_(wifiClient_);
NetworkManager::MqttMessageCallback NetworkManager::mqttCallback_ = nullptr;
bool NetworkManager::apModeActive_ = false;
bool NetworkManager::mdnsRunning_ = false;
String NetworkManager::hostname_ = "";

bool NetworkManager::begin(const char* hostname, uint32_t timeoutSeconds) {
  hostname_ = hostname;

  Serial.printf("Connecting to WiFi with hostname: %s\n", hostname);

  // Try to connect to WiFi
  WiFi.begin();

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - startTime) < (timeoutSeconds * 1000)) {
    delay(100);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.printf("WiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());

    // Start mDNS
    if (MDNS.begin(hostname)) {
      Serial.printf("mDNS responder started: %s.local\n", hostname);
      mdnsRunning_ = true;
    }

    apModeActive_ = false;
    return true;
  }

  Serial.println("");
  Serial.println("WiFi connection failed");
  apModeActive_ = true;
  return false;
}

// cppcheck-suppress unusedFunction ; called from PoolMonitorContext.cpp (cross-TU)
bool NetworkManager::beginMqtt(const char* server, uint16_t port, const char* clientId) {
  mqttClient_.setServer(server, port);

  Serial.printf("Attempting MQTT connection to %s:%u...\n", server, port);

  if (mqttClient_.connect(clientId)) {
    Serial.println("MQTT connected");
    return true;
  } else {
    Serial.printf("MQTT connection failed, rc=%d\n", mqttClient_.state());
    return false;
  }
}

void NetworkManager::loop() {
  if (mqttClient_.connected()) {
    mqttClient_.loop();
  }
}

// cppcheck-suppress unusedFunction ; called from PoolMonitorContext.cpp (cross-TU)
bool NetworkManager::isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

// cppcheck-suppress unusedFunction ; called from PoolMonitorContext.cpp (cross-TU)
bool NetworkManager::isMqttConnected() {
  return mqttClient_.connected();
}

bool NetworkManager::publish(const char* topic, const char* payload, bool retained) {
  if (!mqttClient_.connected()) {
    return false;
  }
  return mqttClient_.publish(topic, payload, retained);
}

bool NetworkManager::subscribe(const char* topic) {
  if (!mqttClient_.connected()) {
    return false;
  }
  return mqttClient_.subscribe(topic);
}

// cppcheck-suppress unusedFunction ; called from PoolMonitorContext.cpp (cross-TU)
void NetworkManager::setMqttCallback(MqttMessageCallback callback) {
  mqttCallback_ = callback;
  mqttClient_.setCallback([](char* topic, byte* payload, unsigned int length) {
    if (mqttCallback_) {
      mqttCallback_(topic, payload, length);
    }
  });
}

// cppcheck-suppress unusedFunction ; called from PoolMonitorContext.cpp (cross-TU)
void NetworkManager::disconnectMqtt() {
  if (mqttClient_.connected()) {
    mqttClient_.disconnect();
  }
}

}  // namespace PoolMonitor

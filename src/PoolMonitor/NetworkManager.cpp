// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file NetworkManager.cpp
 * @brief Implementation of NetworkManager for Pool Monitor.
 */

#include "NetworkManager.hpp"
#include "Config.hpp"

namespace PoolMonitor {

// Initialize static members
WiFiClient NetworkManager::wifiClient_;
PubSubClient NetworkManager::mqttClient_(wifiClient_);
NetworkManager::MqttMessageCallback NetworkManager::mqttCallback_ = nullptr;

bool NetworkManager::beginMqtt(const char* server, uint16_t port, const char* clientId,
                               const char* username, const char* password) {
  mqttClient_.setServer(server, port);

  Serial.printf("Attempting MQTT connection to %s:%u...\n", server, port);

  bool connected = false;
  if (username != nullptr && username[0] != '\0') {
    Serial.printf("  Authenticating as user \"%s\"\n", username);
    connected = mqttClient_.connect(clientId, username, password);
  } else {
    connected = mqttClient_.connect(clientId);
  }

  if (connected) {
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

bool NetworkManager::isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

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

void NetworkManager::setMqttCallback(MqttMessageCallback callback) {
  mqttCallback_ = callback;
  mqttClient_.setCallback([](char* topic, byte* payload, unsigned int length) {
    if (mqttCallback_) {
      mqttCallback_(topic, payload, length);
    }
  });
}

void NetworkManager::disconnectMqtt() {
  if (mqttClient_.connected()) {
    mqttClient_.disconnect();
  }
}

}  // namespace PoolMonitor

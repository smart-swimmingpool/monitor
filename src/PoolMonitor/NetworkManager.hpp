// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file NetworkManager.hpp
 * @brief WiFi and MQTT connection management with AP fallback for Pool Monitor.
 *
 * Manages WiFi connectivity and MQTT broker connection with automatic
 * retry. Uses PubSubClient for compatibility with existing code.
 * Note: For deep-sleep operation, connection management is simplified.
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <functional>
#include "Config.hpp"

namespace PoolMonitor {

/**
 * @brief Manages WiFi connectivity and MQTT broker connection.
 *
 * Implements connection management for the Pool Monitor's deep-sleep
 * operation. Uses PubSubClient (synchronous) instead of AsyncMqttClient
 * to maintain compatibility with the existing WiFiSettings library.
 */
class NetworkManager {
public:
  using MqttMessageCallback = std::function<void(char* topic, byte* payload, unsigned int length)>;

  NetworkManager() = default;

  /**
   * @brief Initialize MQTT connection.
   * @param server MQTT server hostname or IP.
   * @param port MQTT server port.
   * @param clientId MQTT client ID.
   * @param username Optional MQTT username. Empty string or nullptr → anonymous.
   * @param password Optional MQTT password (ignored when username is empty).
   * @return true if MQTT connected successfully.
   */
  static bool beginMqtt(const char* server, uint16_t port, const char* clientId,
                        const char* username = nullptr, const char* password = nullptr);

  /** @brief Maintain WiFi and MQTT connections. */
  static void loop();

  /** @brief Check if WiFi is connected. */
  static bool isWiFiConnected();

  /** @brief Check if MQTT is connected. */
  static bool isMqttConnected();

  /**
   * @brief Publish an MQTT message.
   * @param topic MQTT topic.
   * @param payload Message payload.
   * @param retained Set retained flag.
   * @return true if publish successful.
   */
  static bool publish(const char* topic, const char* payload, bool retained = false);

  /**
   * @brief Subscribe to an MQTT topic.
   * @param topic MQTT topic to subscribe to.
   * @return true if subscribe successful.
   */
  static bool subscribe(const char* topic);

  /**
   * @brief Register MQTT message callback.
   * @param callback Function to call when MQTT message received.
   */
  static void setMqttCallback(MqttMessageCallback callback);

  /** @brief Disconnect from MQTT broker. */
  static void disconnectMqtt();

private:
  static WiFiClient wifiClient_;
  static PubSubClient mqttClient_;
  static MqttMessageCallback mqttCallback_;
};

}  // namespace PoolMonitor

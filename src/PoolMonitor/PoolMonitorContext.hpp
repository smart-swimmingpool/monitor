// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file PoolMonitorContext.hpp
 * @brief Core monitor context — owns all subsystems for the Pool Monitor.
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <WiFiSettings.h>
#include "Config.hpp"

namespace PoolMonitor {

/**
 * @brief Singleton context that initializes and runs all monitor subsystems.
 *
 * Owns display manager, network manager, system monitor, and state management.
 * Uses RAII — constructor builds the context, setup() initializes hardware,
 * loop() runs the monitoring cycle.
 *
 * Designed for deep-sleep operation: the context is recreated on each wakeup.
 */
struct PoolMonitorContext final {
  PoolMonitorContext();
  // no copy
  PoolMonitorContext(const PoolMonitorContext &) = delete;
  // no move
  PoolMonitorContext(PoolMonitorContext &&) = delete;
  // no copy
  auto operator=(const PoolMonitorContext &) -> PoolMonitorContext & = delete;
  // no move
  auto operator=(PoolMonitorContext &&) -> PoolMonitorContext & = delete;
  ~PoolMonitorContext();

  /**
   * @brief Startup the monitor.
   * Calls begin() on all subsystems in dependency order.
   * @note Call from the Arduino setup() function exactly once.
   */
  auto setup() -> void;

  /**
   * @brief Run the main monitoring loop iteration.
   * Handles watchdog feeding, memory checks, MQTT processing, and display updates.
   * @note Call from the Arduino loop() function. For deep-sleep operation,
   * this typically runs once before entering sleep.
   */
  auto loop() -> void;

  /**
   * @brief Prepare for deep sleep.
   * Disconnects MQTT, powers down display, and saves state.
   */
  auto prepareForSleep() -> void;

  /**
   * @brief Check if MQTT is connected.
   */
  static auto isMqttConnected() -> bool;

  /**
   * @brief Check if WiFi is connected.
   */
  static auto isWiFiConnected() -> bool;

  /**
   * @brief Check if NTP time sync is needed based on last sync timestamp.
   * @return true if sync is needed, false otherwise.
   */
  static bool isNtpSyncNeeded();

private:
  /**
   * @brief Initialize display and show initial screen.
   */
  auto initializeDisplay() -> void;

  /**
   * @brief Initialize network connections.
   */
  auto initializeNetwork() -> void;

  /**
   * @brief Show setup screen with QR code for WiFi configuration.
   */
  static void showSetupScreen();

  /**
   * @brief Show screen when WiFi connection is successful.
   */
  static void showWiFiConnectedScreen();

  /**
   * @brief Show screen when WiFi connection fails.
   */
  static void showWiFiConnectionFailedScreen();

  /**
   * @brief Initialize MQTT subscriptions.
   */
  auto initializeMqtt() -> void;

  /**
   * @brief Load saved state from Preferences.
   */
  auto loadState() -> void;

  /**
   * @brief Save current state to Preferences.
   */
  auto saveState() -> void;

  /**
   * @brief Handle incoming MQTT messages.
   */
  static void handleMqttMessage(char* topic, byte* payload, unsigned int length);

  bool bootLoopDetected_ = false;
  bool stateLoaded_ = false;

  // MQTT settings (static for access from callbacks)
  static String mqtt_server;
  static uint16_t mqtt_server_port;
};

}  // namespace PoolMonitor

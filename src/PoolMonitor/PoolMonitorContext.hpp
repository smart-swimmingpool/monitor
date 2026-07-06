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
  PoolMonitorContext(const PoolMonitorContext &) = delete;
  PoolMonitorContext(PoolMonitorContext &&) = delete;
  auto operator=(const PoolMonitorContext &) -> PoolMonitorContext & = delete;
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
  static auto isNtpSyncNeeded() -> bool;

private:
  // ── Setup lifecycle (phases of setup())                                   ──

  static auto printBootBanner() -> void;
  auto initSystem() -> void;
  auto updateBootAndUptimeStats() -> unsigned long;
  auto handleBootLoop(unsigned long totalUptime) -> void;
  auto isNetworkCycle() -> bool;
  auto runNetworkCycle(unsigned long totalUptime) -> void;
  auto runOfflineCycle(unsigned long totalUptime) -> void;

  // ── Subsystem helpers                                                      ──

  auto initializeDisplay() -> void;
  auto initializeNetwork() -> void;
  auto initializeMqtt() -> void;

  static auto showSetupScreen() -> void;
  static auto showWiFiConnectedScreen() -> void;
  static auto showWiFiConnectionFailedScreen() -> void;

  auto loadState() -> void;
  auto saveState() -> void;

  static auto handleMqttMessage(char* topic, byte* payload, unsigned int length) -> void;

  bool bootLoopDetected_ = false;
  bool stateLoaded_ = false;

  static String mqtt_server;
  static uint16_t mqtt_server_port;
};

}  // namespace PoolMonitor

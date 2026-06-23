// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file main.cpp
 * @brief Main entry point for the Pool Monitor firmware.
 *
 * Arduino entry point: setup() initializes all subsystems, loop() runs
 * the monitoring cycle. For deep-sleep operation, loop() typically
 * runs once before entering sleep.
 */

#include <Arduino.h>
#include "PoolMonitor/PoolMonitorContext.hpp"
#include "PoolMonitor/Config.hpp"

/** @brief Singleton context owning all monitor subsystems. */
static PoolMonitor::PoolMonitorContext context{};

/**
 * @brief Arduino setup() — initializes serial and delegates to PoolMonitorContext.
 *
 * Waits up to 3 seconds for a USB serial connection (non-blocking fallback
 * for headless operation), then calls context.setup() to initialize all
 * subsystems.
 */
auto setup() -> void {
  Serial.begin(SERIAL_SPEED);

  // Wait for serial port to connect. Needed for native USB port only.
  // Non-blocking fallback for headless operation.
  const uint32_t startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    delay(10);
  }

  context.setup();
}

/**
 * @brief Arduino loop() — runs the monitoring cycle.
 *
 * Called continuously after setup(). Delegates to context.loop() which
 * handles watchdog feeding, memory checks, MQTT processing, and display updates.
 * For deep-sleep operation, this typically runs once before entering sleep.
 */
auto loop() -> void {
  context.loop();
}

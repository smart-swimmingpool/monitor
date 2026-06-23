// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file SystemMonitor.hpp
 * @brief Watchdog, memory monitor, and boot-loop detection for Pool Monitor.
 *
 * Monitors memory usage and provides watchdog functionality for reliable
 * deep-sleep operation. Adapted from Pool Controller's SystemMonitor.
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <esp_idf_version.h>
#include <esp_task_wdt.h>
#include "Config.hpp"

namespace PoolMonitor {

/**
 * @brief System monitor for deep-sleep based operation.
 *
 * Provides watchdog, memory monitoring, and boot-loop detection
 * specifically adapted for the Pool Monitor's deep-sleep wake cycles.
 */
class SystemMonitor {
private:
  static constexpr uint32_t LOW_MEMORY_THRESHOLD = 16384;
  static constexpr uint32_t CRITICAL_MEMORY_THRESHOLD = 8192;

  static uint32_t lastMemoryCheck;
  static uint32_t minFreeHeap;
  static bool lowMemoryWarning;

public:
  /**
   * @brief Initialize system monitor and watchdog.
   *
   * ESP32 TWDT: 30-second timeout, panic on timeout.
   * Note: For deep-sleep operation, watchdog is less critical as the
   * device will reset anyway, but it's still useful for detecting hangs
   * during the active phase.
   */
  static void begin() {
    lastMemoryCheck = 0;
    minFreeHeap = ESP.getFreeHeap();
    lowMemoryWarning = false;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    const esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 30000,
      .idle_core_mask = 0,
      .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&wdt_config);
#else
    esp_task_wdt_init(30, true);
#endif
    esp_task_wdt_add(NULL);
  }

  /**
   * @brief Feed the watchdog — call this regularly in main loop.
   */
  static void feedWatchdog() { esp_task_wdt_reset(); }

  /**
   * @brief Check memory status and reboot if critically low.
   * @param forceCheck If true, forces a check regardless of interval.
   *
   * Call this periodically (e.g., every 10 seconds) during active operation.
   * For deep-sleep devices, this is mainly useful during the brief active
   * window between wakeup and sleep.
   */
  static void checkMemory(bool forceCheck = false) {
    uint32_t now = millis();

    // Check every 10 seconds, or immediately if forced
    if (!forceCheck && now - lastMemoryCheck < 10000) {
      return;
    }
    lastMemoryCheck = now;

    uint32_t freeHeap = ESP.getFreeHeap();

    // Track minimum heap
    if (freeHeap < minFreeHeap) {
      minFreeHeap = freeHeap;
    }

    // Critical memory — reboot immediately
    if (freeHeap < CRITICAL_MEMORY_THRESHOLD) {
      Serial.printf("CRITICAL: Free heap %d bytes < %d bytes. Rebooting...\n", 
                    freeHeap, CRITICAL_MEMORY_THRESHOLD);
      Serial.flush();
      delay(1000);
      ESP.restart();
    }

    // Low memory — log warning
    if (freeHeap < LOW_MEMORY_THRESHOLD && !lowMemoryWarning) {
      Serial.printf("WARNING: Low memory detected. Free heap: %d bytes "
                    "(min: %d)\n",
        freeHeap, minFreeHeap);
      lowMemoryWarning = true;
    } else if (freeHeap >= LOW_MEMORY_THRESHOLD && lowMemoryWarning) {
      lowMemoryWarning = false;
    }
  }

  /** @brief Get current free heap. */
  static uint32_t getFreeHeap() { return ESP.getFreeHeap(); }

  /** @brief Get minimum free heap since boot. */
  static uint32_t getMinFreeHeap() { return minFreeHeap; }

  /** @brief Force a reboot. */
  static void reboot() {
    Serial.println("System reboot requested");
    Serial.flush();
    delay(1000);
    ESP.restart();
  }

  /** @brief Get uptime in seconds. */
  static uint32_t getUptimeSeconds() { return millis() / 1000; }

  /** @brief Check if system is healthy. */
  static bool isHealthy() { return ESP.getFreeHeap() >= LOW_MEMORY_THRESHOLD; }

  // --- Boot-loop detection ---

  /** @brief Number of consecutive boots before safe mode activates. */
  static constexpr uint8_t BOOT_LOOP_MAX_COUNT = 3;

  /** @brief Minimum uptime (seconds) before clearing the boot-loop counter. */
  static constexpr uint32_t BOOT_LOOP_CLEAR_AFTER_SEC = 300;  // 5 min

  /**
   * @brief Detect boot-loop pattern.
   * @return true when BOOT_LOOP_MAX_COUNT consecutive boots have occurred.
   *
   * Call this as early as possible in setup(), before MQTT/network initializes.
   * Increments a persistent boot counter in NVS on every boot.
   * For deep-sleep devices, this helps detect rapid reboot cycles.
   */
  static bool detectBootLoop() {
    Preferences prefs;
    prefs.begin("sysmon", false);

    int bootCount = prefs.getInt("bootCount", 0) + 1;

    Serial.printf("  Boot counter: %d\n", bootCount);

    bool isBootLoop = (bootCount >= BOOT_LOOP_MAX_COUNT);
    if (isBootLoop) {
      Serial.printf("✖ BOOT-LOOP DETECTED (%d consecutive boots)\n", bootCount);
      Serial.println("  Entering safe mode");
    }

    prefs.putInt("bootCount", bootCount);
    prefs.end();

    return isBootLoop;
  }

  /**
   * @brief Clear the boot-loop counter.
   *
   * Call this after BOOT_LOOP_CLEAR_AFTER_SEC seconds of stable operation
   * to indicate a healthy boot.
   */
  static void clearBootLoopCounter() {
    Preferences prefs;
    prefs.begin("sysmon", false);
    prefs.putInt("bootCount", 0);
    prefs.end();
  }
};

}  // namespace PoolMonitor

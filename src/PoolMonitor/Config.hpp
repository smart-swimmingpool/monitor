// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file Config.hpp
 * @brief Pin assignments and compile-time constants for the Pool Monitor.
 *
 * All GPIO pin assignments and tunable constants are centralized here.
 * Changing these values requires rebuilding the firmware.
 *
 * ## Hardware Variants
 *
 * Currently supports:
 *   **LILYGO_T5_V231** — ESP32 with 2.13" E-Ink display (default)
 *
 * Future variants can be added by defining appropriate preprocessor macros.
 */

#pragma once

#include <cstdint>

namespace PoolMonitor {

/**
 * @brief Interval for display updates (seconds).
 */
constexpr std::uint32_t DISPLAY_UPDATE_INTERVAL{30};

/**
 * @brief Time ESP32 will go to sleep (in seconds).
 */
constexpr std::uint32_t TIME_TO_SLEEP_SECONDS{180};

/**
 * @brief Number of wake cycles to skip WiFi/MQTT to save power.
 *
 * WiFi/MQTT connects only every (SKIP_WIFI_WAKE_CYCLES + 1) wake cycles.
 * Default: 5 → WiFi every ~15 minutes (6 × 180s).
 * Set to 0 to connect every cycle (original behavior).
 */
constexpr std::uint32_t SKIP_WIFI_WAKE_CYCLES{5};

/**
 * @brief NTP sync interval (seconds).
 */
constexpr std::uint32_t NTP_SYNC_INTERVAL_SECONDS{3600};

/**
 * @brief Night mode — reduces wake frequency when nobody watches the display.
 *
 * Between NIGHT_START_HOUR and NIGHT_END_HOUR (local time) the device
 * wakes only every NIGHT_SLEEP_INTERVAL_SECONDS instead of the normal
 * TIME_TO_SLEEP_SECONDS. Saves significant power during the night.
 */
constexpr std::uint32_t NIGHT_START_HOUR{22};
constexpr std::uint32_t NIGHT_END_HOUR{6};
constexpr std::uint32_t NIGHT_SLEEP_INTERVAL_SECONDS{14400};  // 4 hours

/**
 * @brief MQTT payload buffer size for callback handling.
 */
constexpr size_t MQTT_PAYLOAD_BUFFER_SIZE{128};

// ═══════════════════════════════════════════════════════════════════════════
// LILYGO T5 V2.31 — ESP32 with 2.13" E-Ink Display (default)
// ═══════════════════════════════════════════════════════════════════════════

// Default configuration for LILYGO_T5_V231
// These pins match the board_def.h definitions

/** @brief E-Ink display busy pin. */
constexpr std::uint8_t PIN_ELINK_BUSY{4};
/** @brief E-Ink display reset pin. */
constexpr std::uint8_t PIN_ELINK_RESET{16};
/** @brief E-Ink display DC pin. */
constexpr std::uint8_t PIN_ELINK_DC{17};
/** @brief E-Ink display chip select pin. */
constexpr std::uint8_t PIN_ELINK_SS{5};

/** @brief SPI MOSI pin for display. */
constexpr std::uint8_t PIN_SPI_MOSI{23};
/** @brief SPI MISO pin for display (not used). */
constexpr std::int8_t PIN_SPI_MISO{-1};
/** @brief SPI CLK pin for display. */
constexpr std::uint8_t PIN_SPI_CLK{18};

/** @brief Modem power control pin. */
constexpr std::uint8_t PIN_MODEM_POWER_ON{23};

/** @brief Built-in LED pin. */
constexpr std::uint8_t PIN_LED_BUILTIN{2};

/** @brief Device name for mDNS and MQTT client ID. */
constexpr const char* DEVICE_NAME{"pool-monitor"};

/** @brief GitHub repository for OTA updates (provided via -D build flag). */
#ifndef GITHUB_REPO
constexpr const char* GITHUB_REPO{"smart-swimmingpool/monitor"};
#endif

// Serial speed - provided as build flag from platformio.ini
#ifndef SERIAL_SPEED
constexpr std::uint32_t SERIAL_SPEED{115200};
#endif

}  // namespace PoolMonitor

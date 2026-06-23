// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file DisplayManager.hpp
 * @brief E-Ink display management for Pool Monitor.
 *
 * Manages the E-Ink display initialization, static content, and dynamic updates.
 * Supports different display types based on compile-time configuration.
 */

#pragma once

#include <Arduino.h>
#include <GxEPD.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Config.hpp"

namespace PoolMonitor {

/**
 * @brief Manages the E-Ink display for Pool Monitor.
 *
 * Handles display initialization, static content drawing (icons, labels),
 * and dynamic content updates (temperatures, pump status, etc.).
 */
class DisplayManager {
public:
  DisplayManager() = default;

  /**
   * @brief Initialize the display.
   * @return true if initialization successful.
   */
  static bool begin();

  /**
   * @brief Initialize display with static content (icons, labels, lines).
   */
  static void initDisplay();

  /**
   * @brief Update display with current data.
   * @param poolTemp Pool temperature in °C.
   * @param solarTemp Solar temperature in °C.
   * @param poolPumpOn Pool pump status.
   * @param solarPumpOn Solar pump status.
   * @param mode Operation mode string.
   * @param lastUpdate Time string for last update.
   */
  static void updateDisplay(float poolTemp, float solarTemp, bool poolPumpOn, 
                           bool solarPumpOn, const char* mode, const char* lastUpdate);

  /** @brief Power down the display. */
  static void powerDown();

  /** @brief Update full display (not just partial). */
  static void fullUpdate();

  /** @brief Get display width. */
  static uint16_t getWidth();

  /** @brief Get display height. */
  static uint16_t getHeight();

  /** @brief Get reference to the display instance. */
  static auto getDisplay() -> GxEPD::GxEPD_Class&;

  /** @brief Get reference to the u8g2 instance. */
  static auto getU8g2() -> U8G2_FOR_ADAFRUIT_GFX&;

private:
  // Display type selection based on board definition
  #if defined(LILYGO_T5_V231)
  static GxDEPG0213BN display_;
  #else
  static GxGDE0213B1 display_;
  #endif

  static U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx_;

  /**
   * @brief Draw text on display with alignment.
   * @param text Text to draw.
   * @param y Y position (baseline).
   * @param align Alignment (GxEPD_ALIGN_LEFT, RIGHT, CENTER).
   * @param xOffset X offset from alignment position.
   */
  static void displayText(const char* text, int16_t y, uint8_t align = GxEPD_ALIGN_LEFT, 
                         int16_t xOffset = 0);
};

}  // namespace PoolMonitor

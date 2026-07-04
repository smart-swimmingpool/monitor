// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file DisplayManager.hpp
 * @brief E-Ink display management for Pool Monitor.
 *
 * Manages the E-Ink display initialization, static content, and dynamic updates.
 * Supports different display types based on compile-time configuration.
 */

#pragma once

#include <Arduino.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxEPD.h>
#include <SPI.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Config.hpp"

// Include the appropriate display driver based on board definition
#if defined(LILYGO_T5_V231)
#include <GxDEPG0213BN/GxDEPG0213BN.h>
#else
#include <GxGDE0213B1/GxGDE0213B1.h>
#endif

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// U8g2 font declarations for icons
extern const uint8_t u8g2_font_streamline_all_t[] U8G2_FONT_SECTION("u8g2_font_streamline_all_t");
extern const uint8_t u8g2_font_streamline_ecology_t[] U8G2_FONT_SECTION("u8g2_font_streamline_ecology_t");

// Text alignment constants (replaces the anonymous enum from u8g2_display.h)
enum {
  GxEPD_ALIGN_RIGHT,
  GxEPD_ALIGN_LEFT,
  GxEPD_ALIGN_CENTER,
};

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
  static auto getDisplay() -> GxEPD_Class&;

  /**
   * @brief Draw text on display with alignment.
   * @param text Text to draw.
   * @param y Y position (baseline).
   * @param align Alignment (GxEPD_ALIGN_LEFT, RIGHT, CENTER).
   * @param xOffset X offset from alignment position.
   */
  static void displayText(const char* text, int16_t y, uint8_t align = GxEPD_ALIGN_LEFT,
                         int16_t xOffset = 0);

 private:
  static GxIO_Class io_;
  static GxEPD_Class display_;
  static U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx_;

  /**
   * @brief Draw a degree-symbol circle at the given position.
   *
   * Draws three concentric circles to form a degree symbol next to temperature values.
   */
  static void drawDegreeSymbol(int16_t x, int16_t y);

  /**
   * @brief Draw the pump run-status icon at the given position.
   * @param x X position.
   * @param y Y position.
   * @param isOn true draws the "running" icon; false clears the area.
   */
  static void drawPumpIcon(int16_t x, int16_t y, bool isOn);
};

}  // namespace PoolMonitor

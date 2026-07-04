// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file DisplayManager.cpp
 * @brief Implementation of DisplayManager for Pool Monitor.
 */

#include "DisplayManager.hpp"
#include "Config.hpp"
#include "../Version.h"

namespace PoolMonitor {

// Initialize static members
GxIO_Class DisplayManager::io_(SPI, PIN_ELINK_SS, PIN_ELINK_DC, PIN_SPI_CLK);
GxEPD_Class DisplayManager::display_(DisplayManager::io_, PIN_ELINK_RESET,
                                     PIN_ELINK_BUSY);
U8G2_FOR_ADAFRUIT_GFX DisplayManager::u8g2_for_adafruit_gfx_;

bool DisplayManager::begin() {
  Serial.println("🖥️\tInitializing display...");

  display_.init();
  u8g2_for_adafruit_gfx_.begin(display_);

  return true;
}

void DisplayManager::displayText(const char* text, int16_t y, uint8_t align, int16_t xOffset) {
  int16_t x = 0;
  switch (align) {
    case GxEPD_ALIGN_LEFT:
      x = xOffset;
      break;
    case GxEPD_ALIGN_RIGHT:
      x = display_.width() - xOffset;
      break;
    case GxEPD_ALIGN_CENTER:
      x = display_.width() / 2 + xOffset;
      break;
  }

  int16_t x1 = 0, y1 = 0;
  uint16_t textWidth = 0, textHeight = 0;
  display_.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);

  int16_t cursorY = y - static_cast<int16_t>(textHeight) / 2;
  display_.setCursor(x, cursorY);
  display_.print(text);
}

void DisplayManager::initDisplay() {
  Serial.println("🖥️\tInitializing display with static content...");

  display_.fillScreen(GxEPD_WHITE);
  display_.setRotation(3);
  display_.setTextColor(GxEPD_BLACK);

  // Setup u8g2 for icon drawing
  u8g2_for_adafruit_gfx_.setFontMode(0);
  u8g2_for_adafruit_gfx_.setForegroundColor(0);
  u8g2_for_adafruit_gfx_.setBackgroundColor(1);

  // Draw pool icon
  u8g2_for_adafruit_gfx_.setFont(u8g2_font_streamline_all_t);
  u8g2_for_adafruit_gfx_.drawGlyph(4, 50, 0x02a6); /* hex pool */

  display_.setFont(&FreeSans12pt7b);
  displayText("     Pool:", 50, GxEPD_ALIGN_LEFT);

  // Draw solar icon
  u8g2_for_adafruit_gfx_.setFont(u8g2_font_streamline_ecology_t);
  u8g2_for_adafruit_gfx_.drawGlyph(4, 94, 0x003E); /* hex 3E solar panel */

  display_.setFont(&FreeSans12pt7b);
  displayText("     Solar:", 94, GxEPD_ALIGN_LEFT);

  // Draw separator lines
  display_.drawLine(0, 102, display_.width(), 102, GxEPD_BLACK);
  display_.drawLine(0, 103, display_.width(), 103, GxEPD_BLACK);

  display_.setFont(&FreeSans9pt7b);
  displayText("www.smart-swimmingpool.com", 117, GxEPD_ALIGN_LEFT);

  // Firmware version in bottom-right corner
  display_.setFont(&FreeMono9pt7b);
  char versionBuf[16];
  snprintf(versionBuf, sizeof(versionBuf), "v%s", FW_VERSION);
  displayText(versionBuf, 117, GxEPD_ALIGN_RIGHT, 2);

  fullUpdate();
}

void DisplayManager::updateDisplay(float poolTemp, float solarTemp, bool poolPumpOn,
                                   bool solarPumpOn, const char* mode, const char* lastUpdate) {
  Serial.println("🖥️\tUpdating display");

  const int16_t UPDATE_AREA_X = 90;
  const int16_t UPDATE_AREA_Y = 0;
  const int16_t UPDATE_AREA_WIDTH = display_.width() - UPDATE_AREA_X;
  const int16_t UPDATE_AREA_HEIGHT = 95;

  // Buffer for temperature display strings
  char buffer[50];

  display_.fillRect(UPDATE_AREA_X, UPDATE_AREA_Y, UPDATE_AREA_WIDTH, UPDATE_AREA_HEIGHT, GxEPD_WHITE);

  display_.setTextColor(GxEPD_BLACK);
  display_.setFont(&FreeMono9pt7b);
  displayText(mode, 10, GxEPD_ALIGN_CENTER);

  display_.setFont(&FreeMono9pt7b);
  displayText(lastUpdate, 10, GxEPD_ALIGN_RIGHT);

  display_.setTextColor(GxEPD_BLACK);
  display_.setFont(&FreeSansBold24pt7b);

  // Pool temperature
  snprintf(buffer, sizeof(buffer), "%2.1f C", poolTemp);
  displayText(buffer, 50, GxEPD_ALIGN_RIGHT);
  drawDegreeSymbol(display_.width() - 35, 20);

  // Pool pump status icon
  drawPumpIcon(95, 48, poolPumpOn);

  // Solar temperature
  display_.setTextColor(GxEPD_BLACK);
  display_.setFont(&FreeSansBold24pt7b);
  snprintf(buffer, sizeof(buffer), "%2.1f C", solarTemp);
  displayText(buffer, 94, GxEPD_ALIGN_RIGHT);
  drawDegreeSymbol(display_.width() - 35, 64);

  // Solar pump status icon
  drawPumpIcon(95, 94, solarPumpOn);

  display_.updateWindow(UPDATE_AREA_X, UPDATE_AREA_Y, UPDATE_AREA_WIDTH, UPDATE_AREA_HEIGHT, true);
}

void DisplayManager::drawDegreeSymbol(int16_t x, int16_t y) {
  display_.drawCircle(x, y, 5, GxEPD_BLACK);
  display_.drawCircle(x, y, 4, GxEPD_BLACK);
  display_.drawCircle(x, y, 3, GxEPD_BLACK);
}

void DisplayManager::drawPumpIcon(int16_t x, int16_t y, bool isOn) {
  if (isOn) {
    u8g2_for_adafruit_gfx_.setFont(u8g2_font_streamline_all_t);
    u8g2_for_adafruit_gfx_.drawGlyph(x, y, 0x01ec); /* run circle */
  } else {
    display_.fillRect(x, y - 5, 8, 8, GxEPD_WHITE);
  }
}

void DisplayManager::powerDown() {
  display_.powerDown();
}

void DisplayManager::fullUpdate() {
  display_.update();
}

uint16_t DisplayManager::getWidth() {
  return display_.width();
}

uint16_t DisplayManager::getHeight() {
  return display_.height();
}

auto DisplayManager::getDisplay() -> GxEPD_Class& {
  return display_;
}

}  // namespace PoolMonitor

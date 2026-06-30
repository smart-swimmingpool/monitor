// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file TimeClientHelper.hpp
 * @brief NTP time client and timezone support for Pool Monitor.
 */

#pragma once

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>
#include "Config.hpp"

namespace PoolMonitor {

// Define NTP Client to get time
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// Error value for invalid hour (valid range is 0-23)
constexpr uint8_t INVALID_HOUR_VALUE = 255;

// Central European Time (Frankfurt, Paris)
extern TimeChangeRule CEST;
extern TimeChangeRule CET;
extern Timezone CE;

// UTC
extern TimeChangeRule utcRule;
extern Timezone UTC;

extern Timezone currentTZ;

/**
 * @brief Initialize NTP client.
 */
void beginTimeClient();

/**
 * @brief Get current time as formatted string (HH:MM).
 * @return Formatted time string or "--:--" on failure.
 */
String getCurrentTime();

}  // namespace PoolMonitor

// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

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
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};  // Central European Summer Time
TimeChangeRule CET  = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone CE(CEST, CET);

// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};  // UTC
Timezone UTC(utcRule);

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

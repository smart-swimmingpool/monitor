// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file MqttUtils.hpp
 * @brief Utility helpers for MQTT payload parsing in the Pool Monitor.
 */

#pragma once

namespace PoolMonitor {

/**
 * @brief Case-insensitive ASCII string comparison.
 * @return true when both strings are equal (ignoring ASCII case).
 */
bool equalsIgnoreCaseAscii(const char* lhs, const char* rhs);

/**
 * @brief Parse boolean MQTT state payloads used by Home Assistant topics.
 *
 * Accepts "true"/"false" (case-insensitive), "on"/"off", "1"/"0".
 * Logs a warning for unexpected values and defaults to false.
 */
bool parseHomeAssistantBoolState(const char* value);

}  // namespace PoolMonitor

// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter
//
// SPDX-License-Identifier: MIT

/**
 * @file MqttUtils.cpp
 * @brief Implementation of MQTT payload parsing helpers.
 */

#include "MqttUtils.hpp"

#include <Arduino.h>
#include <cctype>

namespace PoolMonitor {

bool equalsIgnoreCaseAscii(const char* lhs, const char* rhs) {
  if (lhs == nullptr || rhs == nullptr) {
    return false;
  }
  while (*lhs != '\0' && *rhs != '\0') {
    if (std::tolower(static_cast<unsigned char>(*lhs)) !=
        std::tolower(static_cast<unsigned char>(*rhs))) {
      return false;
    }
    lhs++;
    rhs++;
  }
  return *lhs == *rhs;
}

bool parseHomeAssistantBoolState(const char* value) {
  if (value == nullptr) {
    return false;
  }
  if (equalsIgnoreCaseAscii(value, "true")
      || equalsIgnoreCaseAscii(value, "on")
      || equalsIgnoreCaseAscii(value, "1")) {
    return true;
  }
  if (equalsIgnoreCaseAscii(value, "false")
      || equalsIgnoreCaseAscii(value, "off")
      || equalsIgnoreCaseAscii(value, "0")) {
    return false;
  }
  Serial.printf("⚠️\tUnexpected boolean MQTT payload: %s (defaulting to false)\n", value);
  return false;
}

}  // namespace PoolMonitor

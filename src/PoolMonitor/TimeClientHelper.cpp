// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file TimeClientHelper.cpp
 * @brief Implementation of NTP time client for Pool Monitor.
 */

#include "TimeClientHelper.hpp"

namespace PoolMonitor {

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");
Timezone currentTZ = CE;

void beginTimeClient() {
  timeClient.begin();
}

String getCurrentTime() {
  // update time with timeout to prevent infinite loop
  int retries = 0;
  const int MAX_RETRIES = 10;
  bool success = false;
  
  while (retries < MAX_RETRIES) {
    if (timeClient.forceUpdate()) {
      success = true;
      break;
    }
    retries++;
    if (retries < MAX_RETRIES) {
      delay(500); // Delay before retry for network operations
    }
  }
  
  if (!success) {
    Serial.println("⚠️\tFailed to update NTP time");
    return String("--:--");
  }
  
  time_t t = currentTZ.toLocal(timeClient.getEpochTime());
  char buf[10];
  snprintf(buf, sizeof(buf), "%.2d:%.2d", hour(t), minute(t));
  
  return String(buf);
}

}  // namespace PoolMonitor

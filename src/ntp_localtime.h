#pragma once

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>

// Define NTP Client to get time
WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

// Error value for invalid hour (valid range is 0-23)
#define INVALID_HOUR_VALUE 255

// see: https://github.com/JChristensen/Timezone/blob/master/examples/WorldClock/WorldClock.ino
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};  // Central European Summer Time
TimeChangeRule CET  = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone       CE(CEST, CET);

// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};  // UTC
Timezone       UTC(utcRule);

Timezone currentTZ = CE;

static String getCurrentTime() {
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
  char   buf[10];
  snprintf(buf, sizeof(buf), "%.2d:%.2d", hour(t), minute(t));

  return String(buf);
}

// Unused function - kept for potential future use
// If this function is needed, consider using cached time instead of forcing NTP sync
// static int getHourOfDay() {
//   // update time with timeout to prevent infinite loop
//   int retries = 0;
//   const int MAX_RETRIES = 10;
//   bool success = false;
//   
//   while (retries < MAX_RETRIES) {
//     if (timeClient.forceUpdate()) {
//       success = true;
//       break;
//     }
//     retries++;
//     if (retries < MAX_RETRIES) {
//       delay(500); // Delay before retry for network operations
//     }
//   }
//   
//   if (!success) {
//     Serial.println("⚠️\tFailed to update NTP time");
//     return INVALID_HOUR_VALUE;
//   }
//   
//   time_t t = currentTZ.toLocal(timeClient.getEpochTime());
//   return hour(t);
// }

// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file SystemMonitor.cpp
 * @brief Implementation of SystemMonitor for Pool Monitor.
 */

#include "SystemMonitor.hpp"

namespace PoolMonitor {

// Initialize static members
uint32_t SystemMonitor::lastMemoryCheck = 0;
uint32_t SystemMonitor::minFreeHeap = 0;
bool SystemMonitor::lowMemoryWarning = false;
Preferences* SystemMonitor::prefs_ = nullptr;

}  // namespace PoolMonitor

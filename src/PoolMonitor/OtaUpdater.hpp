// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file OtaUpdater.hpp
 * @brief OTA firmware update checker and installer — GitHub Releases integration.
 *
 * Designed for Deep-Sleep operation: checks GitHub for new firmware at
 * configurable intervals (tracked across sleep cycles via Preferences).
 * Downloads the firmware *.bin release asset and flashes it via the ESP32
 * Arduino Update library.
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "Config.hpp"

namespace PoolMonitor {

/**
 * @brief Checks GitHub Releases for new firmware, downloads and applies it via OTA.
 *
 * Unlike the pool-controller variant that runs continuously in loop(), this
 * version is designed for deep-sleep wake cycles:
 *  - Call checkForUpdate() during the wake phase (after WiFi is connected).
 *  - Tracks last check time in Preferences (key "last_ota_check") using the
 *    device's total_uptime counter, surviving deep sleep.
 *  - If an update is found, startUpdate() downloads the .bin asset and reboots.
 */
class OtaUpdater {
public:
  OtaUpdater() = default;

  /// Must be called once during setup with an opened Preferences handle.
  static void begin(Preferences &prefs);

  /// Returns true if a newer release was found on GitHub.
  static bool isUpdateAvailable();

  /// Returns true while downloading and flashing.
  static bool isUpdateInProgress();

  /// Current running firmware version (FW_VERSION).
  static String getCurrentVersion();

  /// Latest version tag from GitHub (without "v" prefix).
  static String getLatestVersion();

  /// URL to the GitHub release page.
  static String getReleaseUrl();

  /// Download progress 0–100.
  static int getProgress();

  // ── Actions ──

  /**
   * @brief Check GitHub for a newer release.
   *
   * Call after WiFi + MQTT are connected.  Respects the check interval:
   * if less than kCheckIntervalUptime seconds have passed since the last
   * check, returns false immediately without hitting the network.
   *
   * @param totalUptime Total uptime in seconds (across sleep cycles).
   * @return true if a newer release is available.
   */
  static bool checkForUpdate(unsigned long totalUptime);

  /**
   * @brief Start the OTA download + flash.
   *
   * Assumes WiFi is connected.  On success the ESP32 reboots; on failure
   * the updateAvailable_ flag stays set so the next cycle can retry.
   *
   * @return true if the update started (and reboot will follow).
   */
  static bool startUpdate();

  /// Minimum uptime between GitHub API checks (24 hours in seconds).
  static constexpr unsigned long kCheckIntervalUptime = 86400UL;

  /// Shorter retry interval when a failed OTA attempt should be retried (1 hour).
  static constexpr unsigned long kRetryIntervalUptime = 3600UL;

  /// Max milliseconds without progress before aborting a stalled download.
  static constexpr unsigned long kDownloadTimeoutMs = 30000UL;

private:
  // ── GitHub API ──
  static bool fetchLatestRelease();

  // ── Semver helpers ──
  struct Version {
    int major = 0, minor = 0, patch = 0;
  };
  static bool parseVersion(const String &str, Version &out);
  static bool isNewerVersion(const String &current, const String &latest);

  // ── OTA ──
  static bool downloadAndApply(const String &url);

  // ── State ──
  static Preferences *prefs_;
  static String currentVersion_;
  static String latestVersion_;
  static String releaseUrl_;
  static String downloadUrl_;
  static bool updateAvailable_;
  static bool updateInProgress_;
  static int progress_;

  static constexpr int kOtaBufferSize = 4096;
};

}  // namespace PoolMonitor

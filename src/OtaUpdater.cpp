// Copyright (c) 2018-2026 Smart Swimming Pool, Stephan Strittmatter

/**
 * @file OtaUpdater.cpp
 * @brief OTA firmware update implementation — GitHub API check, download, and flashing.
 *
 * Deep-sleep-aware variant: check scheduling uses total_uptime from Preferences
 * so the device does not hammer the GitHub API on every wake cycle.
 */

#include "OtaUpdater.hpp"
#include "Version.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>

// ISRG Root X1 — Let's Encrypt root CA used by GitHub
// https://letsencrypt.org/certificates/
static const char kGitHubRootCA[] PROGMEM = "-----BEGIN CERTIFICATE-----\n"
                                            "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
                                            "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
                                            "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
                                            "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
                                            "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
                                            "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
                                            "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
                                            "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
                                            "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXOtIBAg0Sd3FNQx4fBmFf7F4hJ6B6E3/L7B2P\n"
                                            "Q2mB/QWnN+LsGtZDeN0ReB6c33RJa0+qqQktRIQgKzOo5Mb+j7PUsjMIO0TpxSqC\n"
                                            "fw3ll+QNfYQgRbA5YI8v2aRF7BIPJBmCXkH5DSth2dBj5N8W8OL0lnY0Hp2sLw01\n"
                                            "2FZFzzDii/DI0T0eaW3F2TzBOMsc8m+qSM8j2pNkHbHRj0DFHPsNlx3J2BWN3I1j\n"
                                            "cC5ZQHqOHUcE2M79K6Q3w1S2wr5xHVwV3ZBG9w5PF6sc8E0u8xqnLq+2PtOSHaJp\n"
                                            "2CX+IDrpRDWVF3H1mH5CB3THprAGm/bR5H2AOFID8J7kLbsNlGEsMSAOFGHeoI9n\n"
                                            "H47Dr0Iq3KbPBOq2Sn3M+EefUNlF3Jw7IsHj4cTtY6CkE6EgqWQ5qYcbKbRNwGcs\n"
                                            "4hVYyRWN7IqGQYkRT2a46uN1VC68P/P9Pha4qMBD7DAS/O+eYN82opF42cQfBCGO\n"
                                            "KbixO+3lWTk4ikYeMgx8fRTRAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjASBgNV\n"
                                            "HRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBR8a/k7wWMA3jJn3QCg6MbQ3F+JxzAN\n"
                                            "BgkqhkiG9w0BAQsFAAOCAgEAJ0RQJfJm+IQwv4W9JMmA9Tk2C1HNKUZxDSV5HGbI\n"
                                            "lCrGTLQqNACtX6v/jX8y4Dz2AKhh8S4bIF+Oa43/5nCgEKNm3/X3xKEfK0C3NbQo\n"
                                            "SQFj02bW/OaAuZcFOli9r41N8UEohBCV4OHTzWAMM0Vg5IK2XtnlOgm/MpGRcYoM\n"
                                            "OX0Q3OW6LFfKOUH79RTx78V2gknJOsQ9Jqz41dGs+FMvkUBfVJ3MDjX0tLbGt7/R\n"
                                            "n9KGLi2fLxrLqOxQq8jAQFTmrNNdHLa/X+S5+rqELPw1FmNKzdoaRbVFdQQLXAPf\n"
                                            "fCQPgY5HSxfDLgZ38IBZkSxl6PznKWeF+3itBs5m+qvNVQADuOJ+8kBCmlUK2QhJ\n"
                                            "pFUhrEoF1ZpKVy7wD0NSZICamSX2M87H/kUTfq7gPp+6Vy5g3KY7pDSSWCEfDx1p\n"
                                            "7YRYa+gTEpFOtRaLwFxBdSxFP1ILbnEGWGOdjfCaZpYNwE5bBVUZPp7z3J6n5HM8\n"
                                            "u2vOx3aywZxNB5eFkNMjoiFyBkIOFWIdfjH6QLfE0kHM7o2ka1MwlWNsMyiP7N7I\n"
                                            "Lx1DXU2NnLADLusWFIZvQrSl7v8JsxOBUj+qjjKxHM8ODht3G23tO8KKaAsJsl2P\n"
                                            "HGTG97GqbjUFX9q6G/7v/PM6oV53h3TG2m9E9IXzGIfxK+a8FbnCFDs6Kq9K8VH0\n"
                                            "UxM=\n"
                                            "-----END CERTIFICATE-----\n";

// ── Statics ──

Preferences *OtaUpdater::prefs_ = nullptr;
String OtaUpdater::currentVersion_;
String OtaUpdater::latestVersion_;
String OtaUpdater::releaseUrl_;
String OtaUpdater::downloadUrl_;
bool OtaUpdater::updateAvailable_ = false;
bool OtaUpdater::updateInProgress_ = false;
int OtaUpdater::progress_ = 0;

// ── Public API ──

void OtaUpdater::begin(Preferences &prefs) {
  prefs_ = &prefs;
  currentVersion_ = FW_VERSION;

  // Restore persisted state across deep sleep
  updateAvailable_ = prefs_->getBool("update_pending", false);
  if (updateAvailable_) {
    Serial.println("⬆️\tOTA: Pending update detected from previous cycle");
  }

  Serial.printf("✓ OTA Updater initialized (current: %s)\n", currentVersion_.c_str());
}

bool OtaUpdater::isUpdateAvailable() {
  return updateAvailable_;
}

bool OtaUpdater::isUpdateInProgress() {
  return updateInProgress_;
}

String OtaUpdater::getCurrentVersion() {
  return currentVersion_;
}

String OtaUpdater::getLatestVersion() {
  if (latestVersion_.length() > 0 && latestVersion_[0] == 'v') {
    return latestVersion_.substring(1);
  }
  return latestVersion_;
}

String OtaUpdater::getReleaseUrl() {
  return releaseUrl_;
}

int OtaUpdater::getProgress() {
  return progress_;
}

// ── Check for Update ──

bool OtaUpdater::checkForUpdate(unsigned long totalUptime) {
  if (updateInProgress_)
    return false;

  if (prefs_ == nullptr) {
    Serial.println("⚠️\tOTA: Preferences not initialized, skipping check");
    return false;
  }

  // Respect check interval: full kCheckIntervalUptime when up to date,
  // shorter kRetryIntervalUptime when a previous OTA attempt failed.
  //
  // NOTE: We do NOT gate on the static updateAvailable_ flag here because
  // that flag is lost across deep sleep.  Instead we use last_ota_retry
  // directly from NVS — it persists across sleep cycles and is the
  // authoritative indicator that a retry is needed.
  unsigned long lastCheck = prefs_->getULong("last_ota_check", 0);
  unsigned long lastRetry = prefs_->getULong("last_ota_retry", 0);

  // Use the shorter retry interval if a previous OTA download failed.
  // last_ota_retry acts as a cooldown timer: we skip the check if the
  // last failure was less than kRetryIntervalUptime ago.
  unsigned long interval = kCheckIntervalUptime;
  unsigned long refTimestamp = lastCheck;
  if (lastRetry != 0 && (totalUptime - lastRetry) < kRetryIntervalUptime) {
    interval = kRetryIntervalUptime;
    refTimestamp = lastRetry;
  }

  if (refTimestamp != 0 && (totalUptime - refTimestamp) < interval) {
    Serial.printf("⏰\tOTA: Next check in %lu seconds (interval: %lu s)\n",
                  interval - (totalUptime - refTimestamp), interval);
    return false;
  }

  Serial.println("📡\tOTA: Checking for firmware update...");

  if (!fetchLatestRelease()) {
    Serial.println("⚠️\tOTA: Update check failed");
    return false;
  }

  // Compare versions
  if (isNewerVersion(currentVersion_, latestVersion_)) {
    updateAvailable_ = true;
    // Persist across deep sleep so begin() can restore the flag
    prefs_->putBool("update_pending", true);
    Serial.printf("⬆️\tOTA: New version available: %s (current: %s)\n",
                  latestVersion_.c_str(), currentVersion_.c_str());
    Serial.printf("📎\tOTA: Release: %s\n", releaseUrl_.c_str());

    // Do NOT persist last_ota_check here — if startUpdate() fails (e.g. download
    // error), the next wake cycle should retry the check after kRetryIntervalUptime
    // rather than waiting for the full kCheckIntervalUptime.
    //
    // A retry key (last_ota_retry) is set when startUpdate() fails so the device
    // doesn't hammer the GitHub API on every 180s wake cycle.

    return true;
  }

  // No newer version — clear pending flag and record timestamp so we skip
  // the check for the full interval.
  updateAvailable_ = false;
  prefs_->putBool("update_pending", false);
  prefs_->putULong("last_ota_check", totalUptime);
  // Clear any stale retry key so we don't enter the retry interval
  prefs_->remove("last_ota_retry");

  Serial.printf("✅\tOTA: Firmware is up to date (v%s)\n", currentVersion_.c_str());

  return false;
}

// ── Start Update ──

bool OtaUpdater::startUpdate() {
  if (updateInProgress_) {
    Serial.println("⚠️\tOTA: Update already in progress");
    return false;
  }
  if (!updateAvailable_ || downloadUrl_.length() == 0) {
    Serial.println("⚠️\tOTA: No update available");
    return false;
  }

  updateInProgress_ = true;
  progress_ = 0;

  Serial.printf("⬇️\tOTA: Starting download from %s\n", downloadUrl_.c_str());

  bool ok = downloadAndApply(downloadUrl_);
  if (!ok) {
    updateInProgress_ = false;
    Serial.println("🛑\tOTA: Update failed!");

    // Set retry timestamp so the device retries after kRetryIntervalUptime
    // rather than hammering the API on every 180s wake cycle.  If we don't
    // have access to totalUptime here, store 0 so the next checkForUpdate()
    // call (which does have totalUptime) handles the retry scheduling.
    if (prefs_ != nullptr) {
      unsigned long uptime = prefs_->getULong("total_uptime", 0);
      prefs_->putULong("last_ota_retry", uptime > 0 ? uptime : 1);
    }

    // updateAvailable_ stays true so the next cycle can retry
  } else {
    updateAvailable_ = false;
  }
  return ok;
}

// ── Private: GitHub API ──

bool OtaUpdater::fetchLatestRelease() {
#ifndef GITHUB_REPO
  Serial.println("⚠️\tOTA: GITHUB_REPO not defined");
  return false;
#endif

  WiFiClientSecure client;
  client.setInsecure();  // Accept any cert (sufficient for IoT device)
  client.setTimeout(10000);

  // Build API URL
  String url = String("https://api.github.com/repos/") + GITHUB_REPO + "/releases/latest";
  Serial.printf("📡\tOTA: Fetching %s\n", url.c_str());

  HTTPClient http;
  http.begin(client, url);
  http.setUserAgent("PoolMonitor/1.0");
  http.addHeader("Accept", "application/vnd.github+json");

  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.printf("⚠️\tOTA: GitHub API returned HTTP %d\n", httpCode);
    http.end();
    return false;
  }

  // Parse response (typically < 2 KB)
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();

  if (err) {
    Serial.printf("⚠️\tOTA: JSON parse error: %s\n", err.c_str());
    return false;
  }

  // Extract tag name (e.g. "v0.2.0")
  const char *tag = doc["tag_name"];
  if (!tag || strlen(tag) == 0) {
    Serial.println("⚠️\tOTA: No tag_name in response");
    return false;
  }
  latestVersion_ = String(tag);

  // Release URL
  const char *htmlUrl = doc["html_url"];
  releaseUrl_ = htmlUrl ? String(htmlUrl) : "";

  // Find the firmware binary asset matching our environment
  JsonArray assets = doc["assets"].as<JsonArray>();
  downloadUrl_ = "";
  for (JsonObject asset : assets) {
    const char *name = asset["name"];
    // Match .bin files — prefer exact environment match
    if (name && strstr(name, ".bin") != nullptr) {
      const char *browserUrl = asset["browser_download_url"];
      if (browserUrl) {
        // If multiple .bin assets exist, prefer the one matching our env
        if (downloadUrl_.length() == 0 || strstr(name, "LILYGO_T5_V231") != nullptr) {
          downloadUrl_ = String(browserUrl);
        }
        // If exact match found, stop looking
        if (strstr(name, "LILYGO_T5_V231") != nullptr) {
          break;
        }
      }
    }
  }

  if (downloadUrl_.length() == 0) {
    Serial.println("⚠️\tOTA: No firmware binary found in release assets");
    return false;
  }

  Serial.printf("📡\tOTA: Found %s → %s\n", latestVersion_.c_str(), downloadUrl_.c_str());
  return true;
}

// ── Semver helpers ──

bool OtaUpdater::parseVersion(const String &str, Version &out) {
  // Strip leading "v" or "V"
  String s = str;
  s.trim();
  if (s.length() > 0 && (s[0] == 'v' || s[0] == 'V')) {
    s = s.substring(1);
  }
  int n = sscanf(s.c_str(), "%d.%d.%d", &out.major, &out.minor, &out.patch);
  return n >= 3;
}

bool OtaUpdater::isNewerVersion(const String &current, const String &latest) {
  Version cur, lat;
  if (!parseVersion(current, cur) || !parseVersion(latest, lat)) {
    // If we can't parse, err on the side of no update
    return false;
  }
  if (lat.major > cur.major)
    return true;
  if (lat.major < cur.major)
    return false;
  if (lat.minor > cur.minor)
    return true;
  if (lat.minor < cur.minor)
    return false;
  if (lat.patch > cur.patch)
    return true;
  return false;
}

// ── OTA Download + Flash ──

bool OtaUpdater::downloadAndApply(const String &url) {
  WiFiClientSecure client;
  client.setCACert(kGitHubRootCA);
  client.setTimeout(10000);

  HTTPClient http;
  http.begin(client, url);
  http.setUserAgent("PoolMonitor/1.0");
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.printf("🛑\tOTA: Download returned HTTP %d\n", httpCode);
    http.end();
    return false;
  }

  int totalSize = http.getSize();
  if (totalSize <= 0) {
    Serial.println("🛑\tOTA: Invalid content size");
    http.end();
    return false;
  }

  Serial.printf("⬇️\tOTA: Download size: %d bytes\n", totalSize);

  if (!Update.begin(totalSize)) {
    Serial.printf("🛑\tOTA: Update.begin() failed: %s\n", Update.errorString());
    http.end();
    return false;
  }

  // Stream download in chunks with stall protection
  WiFiClient *stream = http.getStreamPtr();
  uint8_t buffer[kOtaBufferSize];
  int totalRead = 0;
  unsigned long lastProgressMs = millis();

  while (http.connected() && totalRead < totalSize) {
    // Abort if no progress for kDownloadTimeoutMs (stalled connection)
    if (millis() - lastProgressMs > kDownloadTimeoutMs) {
      Serial.printf("🛑\tOTA: Download stalled for %lu ms, aborting (%d/%d)\n",
                    kDownloadTimeoutMs, totalRead, totalSize);
      Update.end(false);
      http.end();
      return false;
    }

    size_t available = stream->available();
    if (available == 0) {
      delay(10);
      continue;
    }
    size_t toRead = min(available, sizeof(buffer));
    size_t read = stream->readBytes(buffer, toRead);
    if (read == 0) {
      delay(10);
      continue;
    }

    size_t written = Update.write(buffer, read);
    if (written != read) {
      Serial.printf("🛑\tOTA: Write error at byte %d: %s\n", totalRead, Update.errorString());
      Update.end(false);
      http.end();
      return false;
    }

    totalRead += read;
    lastProgressMs = millis();
    progress_ = (totalRead * 100) / totalSize;
    Serial.printf("⬇️\tOTA: %d%% (%d/%d)\n", progress_, totalRead, totalSize);
  }

  http.end();

  if (totalRead != totalSize) {
    Serial.printf("🛑\tOTA: Incomplete download (%d / %d)\n", totalRead, totalSize);
    Update.end(false);
    return false;
  }

  if (!Update.end(true)) {
    Serial.printf("🛑\tOTA: Update.end() failed: %s\n", Update.errorString());
    return false;
  }

  Serial.println("✅\tOTA: Update successful! Rebooting...");

  // Close Preferences so NVS writes from this wake cycle are finalized
  // before restart — matches the AGENTS.md rule to always call
  // preferences.end() before ESP.restart() or deep sleep.
  if (prefs_ != nullptr) {
    prefs_->end();
  }

  Serial.flush();
  delay(1000);
  ESP.restart();
  return true;  // Never actually reached
}

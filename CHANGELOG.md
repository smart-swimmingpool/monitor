# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.1.0](https://github.com/smart-swimmingpool/monitor/compare/v3.0.1...v3.1.0) (2026-08-11)


### Features

* optional MQTT username/password with portal restyling ([9b0ab7a](https://github.com/smart-swimmingpool/monitor/commit/9b0ab7adc46a50ccc266a4e70bba0e459e9b7ca9))


### Bug Fixes

* code review findings — dangling reference, printf, trailing return type ([c6c3bc4](https://github.com/smart-swimmingpool/monitor/commit/c6c3bc4eab7e3e3d28f15c69b2f64a12b6c84e49))
* cpplint header guard style in Version.h (SRC_VERSION_H_) ([685f6e1](https://github.com/smart-swimmingpool/monitor/commit/685f6e1ca5d90731059b0e2b99172d42100ef5e9))
* **license:** add SPDX-License-Identifier to all source files ([61135d6](https://github.com/smart-swimmingpool/monitor/commit/61135d6a8d3dda5d8305c2760cb6c54b33b5f024))

## [Unreleased]

## [3.0.1] - 2026-06-27

### Fixed

- Display ghosting: corrected updateWindow() dimensions (was 1px too small)
- Float printf: added `_printf_float` linker flag for ESP32 temperature display
- NVS data loss: preferences.end() now called before ESP.restart() in SystemMonitor
- OTA security: GitHub API call uses setCACert() instead of setInsecure()
- MQTT polling: replaced fixed-iteration loop with millis()-based timeout (max 500ms)
- Night-mode: use int32_t for secondsUntilEnd to prevent integer underflow

### Changed

- Documented GPIO23 sharing between SPI MOSI and modem power control

## [3.0.0] - 2026-06-26

### Added

- Power optimization: WiFi/MQTT only every 6th wake cycle (SKIP_WIFI_WAKE_CYCLES)
- Power optimization: display update only on actual data changes
- Power optimization: SPIFFS removed (never used, ~200ms saved per cycle)
- OTA update check only when WiFi is active
- Night mode: extended sleep interval (4h) during 22:00–06:00
- Comprehensive development tooling (.editorconfig, .clang-format, CPPLINT.cfg, Makefile)
- GitHub Actions workflows (Super-Linter v8.7.0, PlatformIO CI, release-please v4)
- CODE_OF_CONDUCT.md and CONTRIBUTING.md

### Changed

- MQTT retained-message polling: 200 → 100 iterations (+ safety-net poll)
- Serial wait on boot: 3s → 1s
- Standardized dependencies in platformio.ini (exact versions for NTPClient, Timezone, ArduinoJson)
- Updated GitHub Actions to use actions/checkout@v7
- Updated Super-Linter to v8.7.0

### Fixed

- Boot-loop counter no longer increments on deep-sleep wakes (no false positives)
- Boot-loop counter cleared before safe-mode sleep (next wake exits safe mode)
- Boot-loop counter clear preserved during failed network wakes
- Display no longer cleared on no-network cycles (E-Ink retains image)
- First-boot now immediately starts WiFiSettings portal (no 15-min delay)
- Night-mode sleep duration persisted for correct uptime tracking
- Night-mode sleep clamped to not overshoot NIGHT_END_HOUR
- Dependency version consistency with pool-controller

## [0.1.0] - 2024-01-01

### Features

- Initial release of Pool Monitor
- E-ink display support (LILYGO_T5_V231)
- MQTT integration for pool data
- Home Assistant MQTT Discovery compatibility
- NTP time synchronization
- OTA update functionality

[Unreleased]: https://github.com/smart-swimmingpool/monitor/compare/v3.0.1...HEAD
[3.0.1]: https://github.com/smart-swimmingpool/monitor/compare/v3.0.0...v3.0.1
[3.0.0]: https://github.com/smart-swimmingpool/monitor/compare/v0.1.0...v3.0.0
[0.1.0]: https://github.com/smart-swimmingpool/monitor/releases/tag/v0.1.0

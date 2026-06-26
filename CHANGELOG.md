# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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

[Unreleased]: https://github.com/smart-swimmingpool/monitor/compare/v3.0.0...HEAD
[3.0.0]: https://github.com/smart-swimmingpool/monitor/compare/v0.1.0...v3.0.0
[0.1.0]: https://github.com/smart-swimmingpool/monitor/releases/tag/v0.1.0

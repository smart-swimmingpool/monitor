# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added comprehensive development tooling (.editorconfig, .clang-format, CPPLINT.cfg, Makefile)
- Added GitHub Actions workflows (Super-Linter v8.7.0, PlatformIO CI, release-please v4)
- Added CODE_OF_CONDUCT.md and CONTRIBUTING.md

### Changed

- Standardized dependencies in platformio.ini (exact versions for NTPClient, Timezone, ArduinoJson)
- Updated GitHub Actions to use actions/checkout@v7
- Updated Super-Linter to v8.7.0

### Fixed

- Fixed dependency version consistency with pool-controller

## [0.1.0] - 2024-01-01

### Features

- Initial release of Pool Monitor
- E-ink display support (LILYGO_T5_V231)
- MQTT integration for pool data
- Home Assistant MQTT Discovery compatibility
- NTP time synchronization
- OTA update functionality

[Unreleased]: https://github.com/smart-swimmingpool/monitor/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/smart-swimmingpool/monitor/releases/tag/v0.1.0

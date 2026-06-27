---
title: Software Guide of Pool Monitor
summary: Software development guide for the Pool Monitor — PlatformIO build environment, library dependencies, build configuration, and MQTT topic reference
date: "2026-06-14"
lastmod: "2026-06-14"
draft: false
toc: true
type: docs
featured: true
tags: ["docs", "monitor", "tutorial", "software", "platformio"]
menu:
  docs:
    parent: Pool Monitor
    name: Software Guide
    weight: 30
---

## Development Environment

The Pool Monitor firmware is built with **PlatformIO**, a cross-platform
build system for embedded development.

### Prerequisites

- [Git](https://git-scm.com/)
- [PlatformIO](https://platformio.org/) — install via:

  ```bash
  pip install platformio
  # Or use the VS Code extension
  ```

### Build & Flash

```bash
# Clone the repository
git clone https://github.com/smart-swimmingpool/monitor.git
cd monitor

# Build the firmware
pio run --environment LILYGO_T5_V231

# Flash to the device (auto-detects USB port)
pio run --environment LILYGO_T5_V231 --target upload

# Monitor serial output
pio run --environment LILYGO_T5_V231 --target monitor

# Run static analysis
pio check --environment LILYGO_T5_V231 --skip-packages
```

> **Note**: The first build downloads and compiles all library dependencies
> automatically. This takes 2–5 minutes depending on your internet connection.

### Project Structure

```text
monitor/
├── platformio.ini           # Build configuration
├── Makefile                 # Local dev tasks (lint, build, format)
├── CPPLINT.cfg              # C++ linting config
├── src/
│   ├── main.cpp             # Arduino entry point (setup, loop)
│   └── PoolMonitor/         # Subsystem classes (namespace PoolMonitor)
│       ├── Config.hpp       # Pin definitions & compile-time constants
│       ├── PoolMonitorContext.{hpp,cpp}     # Core context — owns all subsystems
│       ├── DisplayManager.{hpp,cpp}         # E-Ink display management
│       ├── NetworkManager.{hpp,cpp}         # WiFi & MQTT connection
│       ├── OtaUpdater.{hpp,cpp}             # OTA firmware updates
│       ├── SystemMonitor.{hpp,cpp}          # Watchdog, memory, boot-loop detection
│       └── TimeClientHelper.{hpp,cpp}       # NTP time sync & timezone
├── lib/                     # External libraries (managed by PlatformIO)
├── docs/
│   ├── hardware-guide.md    # Hardware assembly guide
│   ├── software-guide.md    # This document
│   └── users-guide.md       # Setup & configuration
├── .github/workflows/       # GitHub Actions CI
└── platformio.ini           # Build configuration
```

---

## Required Libraries

Libraries are declared in [`platformio.ini`](https://github.com/smart-swimmingpool/monitor/blob/main/platformio.ini)
with version pins and resolved automatically by PlatformIO:

- `zinggjm/GxEPD` — E-Ink display driver
- `juerd/ESP-WiFiSettings` — Captive portal for WiFi/MQTT config
- `olikraus/U8g2` — Icon fonts for display
- `olikraus/U8g2_for_Adafruit_GFX` — U8g2 integration with Adafruit GFX
- `knolleary/PubSubClient` — MQTT client
- `arduino-libraries/NTPClient` — NTP time synchronisation
- `jchristensen/Timezone` — Timezone & DST handling
- `adafruit/Adafruit BusIO` — SPI/I2C abstraction
- `adafruit/Adafruit GFX Library` — Graphics primitives
- `bblanchon/ArduinoJson` — JSON parsing for OTA update metadata

Many thanks to the maintainers of these libraries!

---

## Build Configuration

The build configuration is defined in `platformio.ini`.

### Build Defines

| Define | Value | Description |
|--------|-------|-------------|
| `SERIAL_SPEED` | `115200` | Serial monitor baud rate |
| `LILYGO_T5_V231` | `1` | Board variant (see [Hardware Guide](hardware-guide.md#compatible-board-variants)) |

### Run-Time Constants

Defined in `src/PoolMonitor/Config.hpp`:

| Constant | Default | Description |
|----------|---------|-------------|
| `DEVICE_NAME` | `"pool-monitor"` | mDNS hostname and WiFi AP name |
| `TIME_TO_SLEEP_SECONDS` | `180` | Deep sleep duration (seconds) |
| `NTP_SYNC_INTERVAL_SECONDS` | `3600` | Time between NTP synchronizations (1 hour) |
| `MQTT_PAYLOAD_BUFFER_SIZE` | `128` | Max MQTT message payload size (bytes) |

---

## Configuration

### WiFi & MQTT Setup

The firmware uses the **ESP-WiFiSettings** library for the captive portal
configuration. For the **end-user setup process** (connecting to the AP,
entering credentials, QR code portal on error), see the
[Users Guide](users-guide.md).

### MQTT Topics

The Pool Monitor subscribes to **Home Assistant state topics** published by the
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller).
These are fixed topics — no dynamic discovery is used.

| Data | Topic | Payload Example |
|------|-------|----------------|
| Pool water temperature | `homeassistant/sensor/pool-controller/pool-temp/state` | `25.3` |
| Solar collector temperature | `homeassistant/sensor/pool-controller/solar-temp/state` | `55.1` |
| Pool pump status | `homeassistant/switch/pool-controller/pool-pump/state` | `ON` / `OFF` |
| Solar pump status | `homeassistant/switch/pool-controller/solar-pump/state` | `ON` / `OFF` |
| Operation mode | `homeassistant/select/pool-controller/mode/state` | `auto` |

> The monitor only subscribes to these topics — it does **not** publish any
> data itself.

### Preferences (NVS)

> ⚠️ This section is **outdated** — the NVS layout has been refactored into the
> `PoolMonitor` subsystem. See
> [`PoolMonitorContext.cpp`](https://github.com/smart-swimmingpool/monitor/blob/main/src/PoolMonitor/PoolMonitorContext.cpp)
> for the current implementation.

The firmware stores runtime state in ESP32 NVS (Non-Volatile Storage) using
the Arduino `Preferences` library under the `pool-monitor` namespace:

| Key | Type | Purpose |
|-----|------|---------|
| `boot_count` | `uint` | Number of boots (across deep sleep cycles) |
| `total_uptime` | `ulong` | Cumulative uptime across sleep cycles (seconds) |
| `last_ntp_sync` | `ulong` | Uptime value at last NTP sync |
| `last_epoch` | `ulong` | Unix timestamp at last NTP sync |
| `last_update` | `string` | Formatted time string (HH:MM) |
| `pool_temp` | `float` | Last pool water temperature |
| `solar_temp` | `float` | Last solar collector temperature |
| `pump_pool` | `bool` | Pool pump running? |
| `pump_solar` | `bool` | Solar pump running? |
| `pool_mode` | `string` | Pool controller operation mode |

### Time Sync & Display

- **NTP sync** occurs every **3600 seconds** (1 hour). The server
  `europe.pool.ntp.org` is used.
- Between syncs, the current time is **reconstructed** from the stored epoch and
  elapsed uptime.
- The display shows **local time** with automatic daylight saving time
  handling (CET/CEST, configured in `src/PoolMonitor/TimeClientHelper.hpp`).
- The E-Ink display updates **only when data changes** (received via MQTT).

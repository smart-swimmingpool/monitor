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
├── platformio.ini          # Build configuration
├── src/
│   ├── main.cpp            # Main firmware (setup, loop, MQTT, display)
│   ├── board_def.h         # Board-specific pin definitions
│   ├── u8g2_display.h      # E-Ink display helpers (text, icons)
│   └── ntp_localtime.h     # NTP time sync & timezone handling
└── docs/
    ├── hardware-guide.md   # Hardware assembly guide
    ├── software-guide.md   # This document
    ├── users-guide.md      # Setup & configuration
    └── home-assistant-migration.md  # HA migration notes
```

---

## Required Libraries

- adafruit/Adafruit BusIO
- adafruit/Adafruit GFX Library
- zinggjm/GxEPD
- juerd/ESP-WiFiSettings
- olikraus/U8g2
- olikraus/U8g2_for_Adafruit_GFX
- hmueller01/PubSubClient3
- arduino-libraries/NTPClient
- jchristensen/Timezone

Many thanks to maintainers of these libraries!

---

## Build Configuration

The build configuration is defined in `platformio.ini`.

### Build Defines

| Define | Value | Description |
|--------|-------|-------------|
| `SERIAL_SPEED` | `115200` | Serial monitor baud rate |
| `LILYGO_T5_V231` | `1` | Board variant (see [Hardware Guide](hardware-guide.md#compatible-board-variants)) |

### Run-Time Constants

Defined in `src/main.cpp`:

| Constant | Default | Description |
|----------|---------|-------------|
| `DEVICE_NAME` | `"pool-monitor"` | mDNS hostname and WiFi AP name |
| `TIME_TO_SLEEP_SECONDS` | `180` | Deep sleep duration (seconds) |
| `NTP_SYNC_INTERVAL_SECONDS` | `3600` | Time between NTP synchronizations (1 hour) |
| `MQTT_PAYLOAD_BUFFER_SIZE` | `128` | Max MQTT message payload size (bytes) |

---

## Configuration

### WiFi & MQTT Setup

The firmware uses the **ESP-WiFiSettings** library to provide a captive portal
for initial configuration:

1. On first boot (no WiFi configured), the device starts an **access point**
   named `pool-monitor`.
2. Connect to this AP and a web portal opens at `http://192.168.4.1`.
3. Enter:
   - **WiFi SSID** and **password**
   - **MQTT broker hostname** or IP address
   - **MQTT port** (default: 1883)
4. Click **Save** — the device reboots and connects.

See the [Users Guide](users-guide.md) for detailed setup instructions.

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
  handling (CET/CEST, configured in `src/ntp_localtime.h`).
- The E-Ink display updates **only when data changes** (received via MQTT).

---

## Changelog

### 2026-06-10 — DNS failover, mDNS, QR code portal, and 5-minute timeout

- **DNS failover:** MQTT connection is always attempted even when the hostname
  cannot be resolved via DNS. PubSubClient resolves DNS internally.
- **mDNS:** Device registers as `pool-monitor.local` on the local network.
- **WiFi disconnect removed:** Explicit `WiFi.disconnect(true)` before deep
  sleep was removed so the DHCP lease is preserved. The device stays visible in
  the router table.
- **MQTT portal with QR code:** On MQTT failure, a configuration portal starts.
  The display shows SSID, AP IP, and a QR code.
- **5-minute timeout:** The portal stays active for a maximum of 5 minutes.
  Without user interaction, the device enters deep sleep and retries the
  connection on the next wake cycle.
- **upload_speed:** Reduced to 115200 baud for compatibility with older ESP32
  rev1.0 hardware.
- **QR code library:** Uses ESP32 built-in `esp_qrcode` — no additional
  dependency required.

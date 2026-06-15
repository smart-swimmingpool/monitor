---
title: Hardware Guide of Pool Monitor
summary: Step-by-step hardware guide for the Pool Monitor — compatible LILYGO TTGO T5 boards, pin assignment, power supply, and first power-on
date: "2026-06-14"
lastmod: "2026-06-14"
draft: false
toc: true
type: docs
featured: true
tags: ["docs", "hardware", "guide", "e-paper", "display", "esp32", "lilygo"]
menu:
  docs:
    parent: Pool Monitor
    name: Hardware Guide
    weight: 20
---

## Overview

The Pool Monitor is a **ready-to-use display device** built around the
LILYGO TTGO T5 e-paper ESP32 board family. Unlike the
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller),
no additional sensors or relay modules are required — just the board itself.

> **Target audience**: Anyone comfortable with basic electronics (connecting USB
> power). No soldering required.
>
> Total cost: **~35–55€**.

## Safety ⚠️

- The Pool Monitor itself operates at **low voltage (5V USB)** only.
- **However**, it reads data from the
  [Pool Controller](https://github.com/smart-swimmingpool/pool-controller) which
  switches **230V AC mains voltage**. The monitor only receives MQTT data — it
  does **not** directly handle mains voltage.
- Keep the USB cable and board in a dry location. If used outdoors, place it in a
  weatherproof enclosure (IP54 or better).

---

## Required Parts (BOM)

| # | Component | Qty | Approx. Cost | Notes |
| --- | --- | --- | --- | --- |
| 1 | LILYGO TTGO T5 V2.3.1 (V231) E-Paper ESP32 Board | 1 | 25–35€ | 2.13" E-Ink display, built-in ESP32 |
| 2 | USB-C cable (data + power) | 1 | 3–5€ | For flashing firmware and power supply |
| 3 | USB power supply 5V/≥1A | 1 | 5–10€ | Standard phone charger |
| 4 | Optional: enclosure (ABS/PVC, IP54+) | 1 | 5–10€ | For outdoor/splash-prone installation |
| **Total** | | | **~35–55€** | Full kit, no soldering required |

### Where to Buy

All parts are widely available on Amazon, AliExpress, eBay, or at electronics
distributors like Reichelt, Pollin, Conrad (DE/AT/CH).

- **TTGO T5**: Search for "TTGO T5 V2.3.1 e-paper ESP32" or "LILYGO T5
  e-ink display". Make sure to get the **V2.3.1 revision** for the newest
  E-Ink panel.
- **USB-C cable**: Any data-capable cable works. Avoid charge-only cables for
  firmware flashing.
- **Power supply**: Standard USB phone charger (5V/≥1A). The board draws very
  little power, but a quality supply prevents brownouts during WiFi operation.

---

## Compatible Board Variants

The TTGO T5 series comes in multiple revisions. The firmware supports:

| Board Variant | Display Panel | Status | Notes |
|--------------|---------------|--------|-------|
| **T5 V2.3.1 (V231)** | 2.13" GxDEPG0213BN (b/w) | ✅ **Default** | **Recommended** — most recent, actively tested |
| T5 V1.2 / V2.4 | 2.13" GxGDE0213B1 (b/w) | ✅ Supported | Older panel, different driver |
| T5 V2.0 / V2.3 | 2.13" GxGDE0213B1 (b/w) | ✅ Supported | No SD card slot |
| T5 V2.1 | 2.9" GxGDEH029A1 (b/w) | ✅ Supported | Larger 2.9" display |
| T5 V2.2 | 2.9" GxGDEH029A1 (b/w) | ✅ Supported | Different display pinout |
| T5 V2.8 | 2.7" GxGDEW027W3 (b/w) | ✅ Supported | With audio DAC onboard |

To select a different variant, edit `src/board_def.h` and set the corresponding
define at the top of the file:

```cpp
#define LILYGO_T5_V231 1   // default — comment this out for other variants
// #define TTGO_T5_2_1  1  // example: uncomment for 2.9" variant
```

Then rebuild the firmware (see [Software Guide](software-guide.md)).

---

## Pin Assignment

The pin configuration is defined in `src/board_def.h` and varies by board
variant. For the default **V2.3.1 (V231)** board:

| Signal | GPIO | Notes |
| --- | --- | --- |
| E-Ink BUSY | GPIO4 | Display busy output |
| E-Ink RESET | GPIO16 | Display reset line |
| E-Ink DC | GPIO17 | Data/Command control |
| E-Ink SS (CS) | GPIO5 | SPI chip select |
| SPI MOSI | GPIO23 | Display data line |
| SPI MISO | — | Not connected (E-Ink is write-only) |
| SPI CLK | GPIO18 | Display clock |
| MODEM_POWER_ON | GPIO23 | Modem power control (de-asserted in deep sleep) |
| LED_BUILTIN | GPIO2 | Built-in status LED |

> The SPI pins are shared with the ESP32's flash memory — this is normal and
> works because the E-Ink display is only accessed when the CPU is idle.

### Pinout Notes

- **SPI MISO** is unused because the E-Ink display receives data only (no
  read-back needed).
- **GPIO23** is dual-purpose (SPI MOSI + MODEM_POWER_ON). In deep sleep it is
  driven LOW to turn off external modem power. During normal operation both
  functions coexist safely.
- The strapping pins (GPIO0, GPIO2, GPIO12, GPIO15) are avoided for E-Ink
  control signals to prevent boot issues.

---

## Wiring

**No manual wiring is required.** The TTGO T5 board integrates all components:

- ESP32 microcontroller (dual-core Xtensa LX6)
- 2.13" E-Ink display (SPI-connected, GxDEPG0213BN panel)
- USB-to-serial converter (CP210x or CH340)
- 3.3V voltage regulator
- Battery charging circuitry (optional Li-Po connector)

Simply connect the board via **USB-C**:

- To a **computer** for flashing the firmware
- To a **USB phone charger** for permanent operation

### Buttons (on supported variants)

Some TTGO T5 variants include hardware buttons. The firmware currently does
**not** use them — all configuration is done via the captive portal (see
[Users Guide](users-guide.md)).

| Button | GPIO | Notes |
| --- | --- | --- |
| BUTTON_1 | GPIO37 | Present on T5 V2.4/2.8 variants |
| BUTTON_2 | GPIO38 | Present on T5 V2.4/2.8 variants |
| BUTTON_3 | GPIO39 | Present on most variants |

---

## Power Supply

| Source | Voltage | Current | Notes |
| --- | --- | --- | --- |
| USB-C phone charger | 5V | ≥1A | **Recommended** for permanent operation |
| Computer USB port | 5V | ~500mA | Suitable for flashing and testing |
| USB power bank | 5V | varies | Works, but deep sleep is designed for continuous mains power |

### Power Consumption

| State | Current | Duration per cycle |
| --- | --- | --- |
| Active (WiFi + MQTT + display update) | ~80mA | ~10–15 seconds |
| Deep sleep | ~10µA | ~165–170 seconds |
| **Average** | **~5.5mA** | over full 180s cycle |

The 180-second deep sleep cycle means the board spends **>90% of its life in
deep sleep**, making it suitable for battery operation with a suitably sized
Li-Po cell (the TTGO T5 has a built-in battery connector and charger).

---

## First Power-On and Testing

### 1. Visual Inspection

Before connecting power:

- Check the USB-C connector for debris
- Verify the board is not placed on a conductive surface
- Look for any visible damage (swollen components, corrosion)

### 2. Power On

1. Connect the TTGO T5 to a USB power source (computer or phone charger).
2. The E-Ink display will update after a few seconds:
   - **With stock firmware**: a test pattern or blank screen.
   - **With Pool Monitor firmware**: the splash screen — see
     [Users Guide](users-guide.md#setup) for first-time configuration.

### 3. Initial Configuration

The device starts in **AP mode** if no WiFi is configured:

1. Connect your phone/laptop to the **`pool-monitor`** WiFi network (open,
   no password).
2. A captive portal opens automatically — or navigate to
   **`http://192.168.4.1`**.
3. Enter your WiFi credentials and MQTT broker settings.
4. Click **Save** — the device reboots and connects to your network.

Details in the [Users Guide](users-guide.md).

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| Display stays white | No firmware or corrupted flash | Flash firmware via PlatformIO (see [Software Guide](software-guide.md)) |
| "MQTT Error" on display, portal opens | MQTT broker unreachable | Check broker hostname/IP; verify MQTT server is running |
| "WiFi connection failed" | Wrong credentials or weak signal | Reconfigure via captive portal; check WiFi range |
| Display shows old data (never updates) | MQTT issue or pool controller offline | Verify pool controller publishes to HA state topics |
| QR portal appears every 3 minutes | MQTT settings not saving | Re-enter MQTT hostname/IP; try IP address instead of hostname |
| Brownout / reboot loop | Insufficient power supply | Use 5V/≥1A supply; try different USB cable |
| Display updates but shows `--:--` | NTP time sync failed | Check internet connectivity; NTP is attempted every hour |

---

## References

- [LILYGO TTGO T5 Product Page](http://www.lilygo.com/prod_view-66.html)
- [GxEPD Library](https://github.com/ZinggJM/GxEPD)
- [ESP32 Datasheet](https://www.espressif.com/en/products/socs/esp32)
- [Pool Controller Hardware Guide](https://github.com/smart-swimmingpool/pool-controller/blob/main/docs/hardware-guide.md)
- [Board definition source](https://github.com/smart-swimmingpool/monitor/blob/main/src/board_def.h)

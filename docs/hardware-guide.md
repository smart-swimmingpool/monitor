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

## Solar Power Operation ☀️

The Pool Monitor's extremely low power consumption (average ~5.5 mA at 5 V)
makes it an excellent candidate for autonomous solar operation. With a small
solar panel and a Li‑Po battery, the device can run indefinitely without a
USB power supply.

> **Prerequisite**: The TTGO T5 board includes a **built-in Li‑Po charger
> (TP4056)** and a **JST 1.25 mm battery connector**. For solar operation you
> only need a solar panel and a battery — the board handles charging electronics
> itself.

---

### Power Budget

| State | Current | Duration per cycle |
| --- | --- | --- |
| Active (WiFi + MQTT + display update) | ~80 mA | ~10–15 seconds |
| Deep sleep | ~10 µA | ~165–170 seconds |
| **Average** | **~5.5 mA** | over 180 s cycle |
| **Daily energy requirement** | **~132 mAh** | at 5 V (≙ 660 mWh) |

The daily energy requirement of **~130 mAh** (at 5 V) can be met by even a
small solar panel at most locations.

---

### Additional Parts Required

| # | Component | Qty | Approx. Cost | Notes |
| --- | --- | --- | --- | --- |
| 1 | Solar panel, 5 V / 1–2 W | 1 | 10–25 € | Monocrystalline, open-circuit voltage 5.5–6 V |
| 2 | Li‑Po battery, 3.7 V, 1000–2000 mAh | 1 | 8–15 € | With JST 1.25 mm connector (PHR‑2, 2‑pin) |
| 3 | USB‑C breakout board or **XC6206‑5.0V** regulator module | 1 | 2–5 € | For stable 5 V feed from solar panel |
| 4 | Optional: Schottky diode (1N5819) | 1 | 1 € | Reverse current protection when panel is dark |
| 5 | Optional: Enclosure (IP65+, transparent cover) | 1 | 10–20 € | Solar panel can replace lid or be integrated |
| **Total** | — | — | **~30–65 €** | Additional to the base BOM |

#### Where to Buy Solar Components

- **Solar panel**: Search for *"5V 2W solar panel"* or *"6V 200mA solar
  module"*, e.g. on Amazon, AliExpress. Monocrystalline panels with ETFE
  coating are ideal for outdoor use.
- **Li‑Po battery**: Search for *"3.7V 1000mAh Li‑Po JST 1.25"* or
  *"LP401230 1000mAh"*. Make sure it has a **JST PHR‑2 connector
  (1.25 mm pitch)**. Common form factors: 401230 (1000 mAh, ~30×12×4 mm) or
  503048 (1500 mAh).
- **USB‑C breakout**: For direct 5 V injection via the USB‑C port.

---

### Circuit Variants

#### Variant A: Solar Panel → USB‑C (recommended, simple)

The simplest and safest method — uses the **built-in TP4056 charger** of the
TTGO T5.

```text
  Solar Panel          TTGO T5 V231
  (5-6V)                  ┌────────┐
     │                    │ USB-C  │
     ├─[USB-C Breakout]───┤  Port  │
     │                    │        │
  Li‑Po Battery           │  JST   │
  (3.7V, 1000mAh) ────────┤  BAT   │
                           └────────┘
```

1. **Solar panel** (5–6 V) — solder to a USB‑C breakout board or directly to
   a USB‑C plug (VBUS to pins A4/A9/B4/B9, GND to A1/B1).
2. Plug the USB‑C breakout into the TTGO T5's USB‑C port.
3. **Li‑Po battery** — connect to the JST 1.25 mm (BAT) connector on the
   TTGO T5.

*Note*: The panel must deliver **5 V (nominal) up to 6 V max**. The TP4056
requires at least ~4.5 V to start charging.

#### Variant B: External Solar Charger (more flexible)

Uses a dedicated solar charger module (e.g. **CN3065** or **TP4056 with solar
input**) for better energy harvesting in low light.

```text
  Solar Panel        Solar Charger        TTGO T5 V231
  (5-6V)             Module               ┌────────┐
     │                  │                  │ USB-C  │
     ├──────────────────┤ IN+         OUT+─┤  Port  │
     │                  │                  │        │
                         │ BAT+  ────────   │  JST   │
  Li‑Po Battery ────────┤ BAT-      ──────┤  BAT   │
                         │                  └────────┘
```

1. Connect solar panel to the **IN+ / IN−** terminals of the charger module.
2. Connect Li‑Po battery to **BAT+ / BAT−** of the charger.
3. Connect charger output to the TTGO T5's JST BAT connector **or** via a
   5 V step-up converter to the USB‑C port.

> **Advantage**: Modules like the CN3065 feature an **MPPT‑like input
> regulator** that still harvests power in cloudy conditions.

---

### Minimum Configuration (Summer Only) ☀️

If the monitor only needs to run **during the warm season** (May–September)
in **mostly good weather**, minimal components are sufficient:

| Component | Minimum | Rationale |
| --- | --- | --- |
| **Solar panel** | **0.3–0.5 W** (5 V, ~60–100 mA) | Summer yield ~250 mAh/day – twice the daily need |
| **Battery** | **500 mAh** (e.g. LP401015) | Bridges the night (~66 mAh) + 2 cloudy days |
| **Schottky diode** | Optional but recommended | Prevents reverse current at night with small panels |

The **daily requirement of ~130 mAh** is already exceeded by a **0.3 W panel**
(60 mA × 6 h × 0.7 efficiency ≈ 250 mAh/day) in summer. The battery handles
the night – even after a cloudy day there's enough reserve for the next day.

> **Practical tip**: A panel below 0.5 W is often barely cheaper than a
> 0.5 W panel. Recommendation: **0.5 W + 500 mAh battery** as the cheapest
> combination (~15–25 € additional to the board).

### Sizing

#### Battery Capacity

| Autonomy | Recommended Capacity | Example |
| --- | --- | --- |
| Night + 1 rainy day (summer minimum) | 500 mAh | LP401015 / 501215 (~15×12×4 mm) |
| 2–3 days (safety margin) | 1000 mAh | LP401230 (30×12×4 mm) |
| 5–7 days (winter/overcast) | 2000 mAh | LP503048 (50×30×5 mm) |
| 10+ days (emergency reserve) | 3000 mAh | LP604560 (60×45×6 mm) |

**Rule of thumb**: In central Europe (DE/AT/CH) winter months typically see
**2–3 effective sun hours per day** (Nov–Feb). A 1000 mAh battery bridges
5–7 days without sun. A 2000 mAh battery lasts 10–14 days.

#### Solar Panel Size

| Panel | Voltage | Current (max) | Daily yield (winter) | Daily yield (summer) |
| --- | --- | --- | --- | --- |
| 0.3 W | 5 V | ~60 mA | ~30 mAh | ~150 mAh |
| **0.5 W** | **5 V** | **~100 mA** | **~50 mAh** | **~250 mAh** |
| 1 W | 5 V | ~200 mA | ~100 mAh | ~500 mAh |
| 2 W | 5 V | ~400 mA | ~200 mAh | ~1000 mAh |

**Recommendation**:

- **Summer only**: **0.5 W + 500 mAh battery** – small, light, cheap
- **Year-round (DE/AT/CH)**: **1–2 W + 1000–2000 mAh battery**

#### Panel Dimensions (approximate)

| Power | Typical size (W × H) | Thickness |
| --- | --- | --- |
| 0.5 W | 90 × 60 mm | 2–3 mm |
| 1 W | 110 × 70 mm | 2–3 mm |
| 2 W | 155 × 85 mm | 2–3 mm |
| 3 W | 185 × 95 mm | 2–3 mm |

---

### Wiring – Step by Step

#### Preparation

1. **Disconnect TTGO T5 from any power source** (USB‑C and battery).
2. Lay out all components on a non-conductive surface.
3. If using stranded wire: tin the ends, prepare heatshrink tubing.

#### Variant A (recommended) – Step by Step

1. **Prepare the USB‑C breakout**: Tin the VBUS and GND pads with a fine
   soldering iron.
2. **Prepare the solar panel**: Insulate the panel leads with heatshrink;
   positive (red) to VBUS, negative (black) to GND on the breakout.
3. **Optional: Schottky diode 1N5819** — place in series with the positive
   lead (cathode toward breakout, anode toward panel) to prevent reverse
   current at night.
4. **Insert the breakout**: Gently plug the USB‑C breakout into the TTGO T5's
   USB‑C port.
5. **Connect the Li‑Po battery**: Plug the battery connector into the
   **JST 1.25 mm (BAT)** socket on the TTGO T5. Watch polarity — red wire =
   positive.
6. **Visual inspection**: No loose wires, no short circuits.

#### Test

1. Connect **solar panel only** (no battery) — the red LED on the TTGO T5
   should light if the panel receives sufficient light. The board should boot.
2. Disconnect panel, connect **battery only** — the board should boot from
   battery.
3. **Connect both** — normal operation. The TP4056 charges the battery in
   sunlight and automatically switches to battery when dark.
4. Test the firmware: WiFi connection, MQTT, display update.

---

### Enclosure and Placement

- Use a **weatherproof enclosure (IP65+)**, e.g. ABS plastic enclosures from
  Bopla, Spelsberg, or Hammond.
- The **solar panel** can either:
  - be **mounted on the outside** of the enclosure (cable routed through a
    cable gland),
  - or be placed **inside behind a clear window** if the lid is transparent
    (yield reduction ~10–20 %).
- **Placement**: South-facing, tilt 30–45°, no shading (trees/buildings). In
  DE/AT/CH, as little as **2–3 h of midday sun** is enough for minimum
  operation.
- **Ventilation slots** are not required due to the low heat dissipation.
- **Temperatures**: The Li‑Po battery should not exceed 60 °C. In direct
  sunlight on a dark enclosure, avoid heat buildup (use a light-colored
  enclosure or place in partial shade).

---

### Notes and Safety ⚠️

- **Li‑Po safety**: Only use batteries with **built-in overcharge/overdischarge
  protection (PCB)**. Replace immediately if damaged (swelling, bulging).
- **Never** connect the solar panel to the **JST BAT socket** — that connector
  is for the **battery only**. The TP4056 input is limited to 6 V max; higher
  voltage will destroy the charger.
- **USB‑C data path**: Variant A occupies the USB‑C port. To flash new
  firmware, unplug the breakout — unless you use **UART via the pin headers**
  (RX/TX) on the board.
- **Strain relief**: Use cable ties or glue dots inside the enclosure to
  prevent force on the connectors.

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

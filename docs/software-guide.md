---
title: Software Guide of Pool Monitor
summary:
date: "2022-06-11"
lastmod: "2022-06-11"
draft: false
toc: true
type: docs
featured: true
tags: ["docs", "monitor", "tutorial"]
menu:
  docs:
    parent: Pool Monitor
    name: Software Guide
    weight: 30
---

## Development Environment

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

## Defines

## Configuration

## Changelog

### 2026-06-10 — DNS failover, mDNS, QR code portal, and 5-minute timeout

- **DNS failover:** MQTT connection is always attempted even when the hostname cannot be resolved via DNS. PubSubClient resolves DNS internally.
- **mDNS:** Device registers as `pool-monitor.local` on the local network.
- **WiFi disconnect removed:** Explicit `WiFi.disconnect(true)` before deep sleep was removed so the DHCP lease is preserved. The device stays visible in the router table.
- **MQTT portal with QR code:** On MQTT failure, a configuration portal starts. The display shows SSID, AP IP, and a QR code.
- **5-minute timeout:** The portal stays active for a maximum of 5 minutes. Without user interaction, the device enters deep sleep and retries the connection on the next wake cycle.
- **upload_speed:** Reduced to 115200 baud for compatibility with older ESP32 rev1.0 hardware.
- **QR code library:** Uses ESP32 built-in `esp_qrcode` — no additional dependency required.

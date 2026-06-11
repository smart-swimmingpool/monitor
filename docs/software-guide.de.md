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

### 2026-06-10 — DNS-Failover, mDNS, QR-Code-Portal und 5-Minuten-Timeout

- **DNS-Failover:** MQTT-Verbindung wird immer versucht, auch wenn der Hostname nicht via DNS aufgelöst werden kann. PubSubClient löst DNS selbst auf.
- **mDNS:** Gerät registriert sich als `pool-monitor.local` im Netzwerk.
- **WiFi-Disconnect entfernt:** Das explizite `WiFi.disconnect(true)` vor dem Tiefschlaf wurde entfernt, damit der DHCP-Lease erhalten bleibt. Das Gerät bleibt in der Router-Tabelle sichtbar.
- **MQTT-Portal mit QR-Code:** Bei MQTT-Fehler startet ein Konfigurationsportal. Display zeigt SSID, AP-IP und QR-Code.
- **5-Minuten-Timeout:** Das Portal bleibt maximal 5 Minuten aktiv. Ohne Benutzereingabe geht das Gerät in Tiefschlaf und wiederholt den Verbindungsversuch beim nächsten Wake.
- **upload_speed:** Auf 115200 Baud reduziert für Kompatibilität mit älteren ESP32 rev1.0.
- **QR-Code Bibliothek:** Verwendet ESP32-Builtin `esp_qrcode` — keine zusätzliche Abhängigkeit.

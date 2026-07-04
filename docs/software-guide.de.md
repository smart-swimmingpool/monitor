---
title: Software Guide des Pool Monitors
summary: Software-Entwicklungsanleitung für den Pool Monitor — PlatformIO Build-Umgebung, Bibliotheksabhängigkeiten, Build-Konfiguration und MQTT-Topic-Referenz
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

## Entwicklungsumgebung

Die Pool Monitor Firmware wird mit **PlatformIO** gebaut, einem
plattformübergreifenden Build-System für Embedded-Entwicklung.

### Voraussetzungen

- [Git](https://git-scm.com/)
- [PlatformIO](https://platformio.org/) — Installation:

  ```bash
  pip install platformio
  # Oder die VS Code Extension verwenden
  ```

### Bauen & Flashen

```bash
# Repository klonen
git clone https://github.com/smart-swimmingpool/monitor.git
cd monitor

# Firmware bauen
pio run --environment LILYGO_T5_V231

# Auf das Gerät flashen (USB-Port wird automatisch erkannt)
pio run --environment LILYGO_T5_V231 --target upload

# Serielle Ausgabe überwachen
pio run --environment LILYGO_T5_V231 --target monitor

# Statische Code-Analyse
pio check --environment LILYGO_T5_V231 --skip-packages
```

> **Hinweis**: Der erste Build lädt alle Bibliotheksabhängigkeiten automatisch
> herunter und kompiliert sie. Dies dauert je nach Internetverbindung 2–5
> Minuten.

### Projektstruktur

```text
monitor/
├── platformio.ini           # Build-Konfiguration
├── Makefile                 # Lokale Dev-Tasks (lint, build, format)
├── CPPLINT.cfg              # C++ Linting-Konfiguration
├── src/
│   ├── main.cpp             # Arduino-Entry-Point (setup, loop)
│   └── PoolMonitor/         # Subsystem-Klassen (Namespace PoolMonitor)
│       ├── Config.hpp       # Pin-Definitionen & Compile-Zeit-Konstanten
│       ├── PoolMonitorContext.{hpp,cpp}     # Core-Kontext — besitzt alle Subsysteme
│       ├── DisplayManager.{hpp,cpp}         # E-Ink Display-Verwaltung
│       ├── NetworkManager.{hpp,cpp}         # WiFi & MQTT-Verbindung
│       ├── OtaUpdater.{hpp,cpp}             # OTA-Firmware-Updates
│       ├── SystemMonitor.{hpp,cpp}          # Watchdog, Speicher, Boot-Loop-Erkennung
│       └── TimeClientHelper.{hpp,cpp}       # NTP-Zeitsync & Zeitzone
├── lib/                     # Externe Bibliotheken (von PlatformIO verwaltet)
├── docs/
│   ├── hardware-guide.md    # Hardware-Anleitung
│   ├── software-guide.md    # Dieses Dokument
│   └── users-guide.md       # Einrichtung & Konfiguration
├── .github/workflows/       # GitHub Actions CI
└── platformio.ini           # Build-Konfiguration
```

---

## Benötigte Bibliotheken

Die Bibliotheken werden in [`platformio.ini`](https://github.com/smart-swimmingpool/monitor/blob/main/platformio.ini)
mit Versionsangaben deklariert und von PlatformIO automatisch aufgelöst:

- `zinggjm/GxEPD` — E-Ink Display-Treiber
- `juerd/ESP-WiFiSettings` — Captive Portal für WLAN/MQTT-Konfiguration
- `olikraus/U8g2` — Icon-Schriftarten fürs Display
- `olikraus/U8g2_for_Adafruit_GFX` — U8g2-Integration mit Adafruit GFX
- `knolleary/PubSubClient` — MQTT-Client
- `arduino-libraries/NTPClient` — NTP-Zeitsynchronisation
- `jchristensen/Timezone` — Zeitzone & Sommerzeit
- `adafruit/Adafruit BusIO` — SPI/I2C-Abstraktion
- `adafruit/Adafruit GFX Library` — Grafik-Primitive
- `bblanchon/ArduinoJson` — JSON-Parsing für OTA-Update-Metadaten

Vielen Dank an die Maintainer dieser Bibliotheken!

---

## Build-Konfiguration

Die Build-Konfiguration ist in `platformio.ini` definiert.

### Build-Defines

| Define | Wert | Beschreibung |
|--------|------|-------------|
| `SERIAL_SPEED` | `115200` | Serielle Baudrate (Monitor) |
| `LILYGO_T5_V231` | `1` | Board-Variante (siehe [Hardware Guide](hardware-guide.de.md#kompatible-board-varianten)) |

### Laufzeit-Konstanten

Definiert in `src/PoolMonitor/Config.hpp`:

| Konstante | Standard | Beschreibung |
|-----------|----------|-------------|
| `DEVICE_NAME` | `"pool-monitor"` | mDNS-Hostname und WiFi-AP-Name |
| `TIME_TO_SLEEP_SECONDS` | `180` | Deep-Sleep-Dauer (Sekunden) |
| `NTP_SYNC_INTERVAL_SECONDS` | `3600` | Zeit zwischen NTP-Synchronisationen (1 Stunde) |
| `MQTT_PAYLOAD_BUFFER_SIZE` | `128` | Maximale MQTT-Nachrichtengröße (Bytes) |

---

## Konfiguration

### WiFi & MQTT Einrichtung

Die Firmware verwendet die Bibliothek **ESP-WiFiSettings** für das Captive-
Portal. Für den **Endbenutzer-Setup-Prozess** (Verbinden mit dem AP, Eingabe
der Zugangsdaten, QR-Code-Portal bei Fehlern) siehe den
[Users Guide](users-guide.de.md).

### MQTT Topics

Der Pool Monitor abonniert **Home Assistant State Topics**, die vom
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller)
veröffentlicht werden. Es werden feste Topics verwendet — keine dynamische
Discovery.

| Daten | Topic | Beispiel-Payload |
|-------|-------|-----------------|
| Pool-Wassertemperatur | `homeassistant/sensor/pool-controller/pool-temp/state` | `25.3` |
| Solar-Kollektortemperatur | `homeassistant/sensor/pool-controller/solar-temp/state` | `55.1` |
| Pool-Pumpen-Status | `homeassistant/switch/pool-controller/pool-pump/state` | `ON` / `OFF` |
| Solar-Pumpen-Status | `homeassistant/switch/pool-controller/solar-pump/state` | `ON` / `OFF` |
| Betriebsmodus | `homeassistant/select/pool-controller/mode/state` | `auto` |

> Der Monitor abonniert nur diese Topics — er veröffentlicht **keine** eigenen
> Daten.

### Preferences (NVS)

> ⚠️ Dieser Abschnitt ist **veraltet** — das NVS-Layout wurde in die
> `PoolMonitor`-Subsysteme überführt. Siehe
> [`PoolMonitorContext.cpp`](https://github.com/smart-swimmingpool/monitor/blob/main/src/PoolMonitor/PoolMonitorContext.cpp)
> für die aktuelle Implementierung.

Die Firmware speichert Betriebszustände im ESP32 NVS (Non-Volatile Storage)
über die Arduino `Preferences`-Bibliothek im Namespace `pool-monitor`:

| Schlüssel | Typ | Zweck |
|-----------|-----|-------|
| `boot_count` | `uint` | Anzahl der Boots (über Deep-Sleep-Zyklen hinweg) |
| `total_uptime` | `ulong` | Kumulierte Betriebszeit (Sekunden) |
| `last_ntp_sync` | `ulong` | Betriebszeit zum Zeitpunkt der letzten NTP-Synchronisation |
| `last_epoch` | `ulong` | Unix-Timestamp bei letztem NTP-Sync |
| `last_update` | `string` | Formatierte Uhrzeit (HH:MM) |
| `pool_temp` | `float` | Letzte Pool-Wassertemperatur |
| `solar_temp` | `float` | Letzte Solar-Kollektortemperatur |
| `pump_pool` | `bool` | Pool-Pumpe läuft? |
| `pump_solar` | `bool` | Solar-Pumpe läuft? |
| `pool_mode` | `string` | Betriebsmodus des Pool Controllers |

### Zeitsynchronisation & Display

- **NTP-Synchronisation** erfolgt alle **3600 Sekunden** (1 Stunde). Der Server
  `europe.pool.ntp.org` wird verwendet.
- Zwischen den Synchronisationen wird die aktuelle Uhrzeit aus dem
  gespeicherten Epoch-Wert und der vergangenen Betriebszeit **rekonstruiert**.
- Das Display zeigt die **Ortszeit** mit automatischer
  Sommerzeitumstellung (CET/CEST, konfiguriert in `src/PoolMonitor/TimeClientHelper.hpp`).
- Das E-Ink Display aktualisiert sich **nur bei Datenänderungen** (per MQTT).

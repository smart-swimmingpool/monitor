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
├── platformio.ini          # Build-Konfiguration
├── src/
│   ├── main.cpp            # Hauptfirmware (setup, loop, MQTT, Display)
│   ├── board_def.h         # Board-spezifische Pin-Definitionen
│   ├── u8g2_display.h      # E-Ink Display-Hilfsfunktionen (Text, Icons)
│   └── ntp_localtime.h     # NTP-Zeitsynchronisation & Zeitzone
└── docs/
    ├── hardware-guide.md   # Hardware-Anleitung
    ├── software-guide.md   # Dieses Dokument
    ├── users-guide.md      # Einrichtung & Konfiguration
    └── home-assistant-migration.md  # HA-Migrationsnotizen
```

---

## Benötigte Bibliotheken

- adafruit/Adafruit BusIO
- adafruit/Adafruit GFX Library
- zinggjm/GxEPD
- juerd/ESP-WiFiSettings
- olikraus/U8g2
- olikraus/U8g2_for_Adafruit_GFX
- hmueller01/PubSubClient3
- arduino-libraries/NTPClient
- jchristensen/Timezone

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

Definiert in `src/main.cpp`:

| Konstante | Standard | Beschreibung |
|-----------|----------|-------------|
| `DEVICE_NAME` | `"pool-monitor"` | mDNS-Hostname und WiFi-AP-Name |
| `TIME_TO_SLEEP_SECONDS` | `180` | Deep-Sleep-Dauer (Sekunden) |
| `NTP_SYNC_INTERVAL_SECONDS` | `3600` | Zeit zwischen NTP-Synchronisationen (1 Stunde) |
| `MQTT_PAYLOAD_BUFFER_SIZE` | `128` | Maximale MQTT-Nachrichtengröße (Bytes) |

---

## Konfiguration

### WiFi & MQTT Einrichtung

Die Firmware verwendet die Bibliothek **ESP-WiFiSettings** für ein Captive
Portal zur Ersteinrichtung:

1. Beim ersten Start (kein WiFi konfiguriert) startet das Gerät einen
   **Access Point** namens `pool-monitor`.
2. Mit diesem AP verbinden — ein Webportal öffnet sich unter
   `http://192.168.4.1`.
3. Eingabe:
   - **WLAN-SSID** und **Passwort**
   - **MQTT-Broker-Hostname** oder IP-Adresse
   - **MQTT-Port** (Standard: 1883)
4. **Save** klicken — das Gerät startet neu und verbindet sich.

Siehe [Users Guide](users-guide.de.md) für detaillierte Anweisungen.

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
  Sommerzeitumstellung (CET/CEST, konfiguriert in `src/ntp_localtime.h`).
- Das E-Ink Display aktualisiert sich **nur bei Datenänderungen** (per MQTT).

---

## Changelog

### 2026-06-10 — DNS-Failover, mDNS, QR-Code-Portal und 5-Minuten-Timeout

- **DNS-Failover:** MQTT-Verbindung wird immer versucht, auch wenn der
  Hostname nicht via DNS aufgelöst werden kann. PubSubClient löst DNS selbst
  auf.
- **mDNS:** Gerät registriert sich als `pool-monitor.local` im Netzwerk.
- **WiFi-Disconnect entfernt:** Das explizite `WiFi.disconnect(true)` vor dem
  Tiefschlaf wurde entfernt, damit der DHCP-Lease erhalten bleibt. Das Gerät
  bleibt in der Router-Tabelle sichtbar.
- **MQTT-Portal mit QR-Code:** Bei MQTT-Fehler startet ein
  Konfigurationsportal. Display zeigt SSID, AP-IP und QR-Code.
- **5-Minuten-Timeout:** Das Portal bleibt maximal 5 Minuten aktiv. Ohne
  Benutzereingabe geht das Gerät in Tiefschlaf und wiederholt den
  Verbindungsversuch beim nächsten Wake.
- **upload_speed:** Auf 115200 Baud reduziert für Kompatibilität mit älteren
  ESP32 rev1.0.
- **QR-Code Bibliothek:** Verwendet ESP32-Builtin `esp_qrcode` — keine
  zusätzliche Abhängigkeit.

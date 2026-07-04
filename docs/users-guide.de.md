---
title: Users Guide des Pool Monitors
summary: Pool Monitor einrichten und konfigurieren — WLAN, MQTT-Broker, Captive Portal, Display-Layout und OTA-Updates
date: "2022-06-11"
lastmod: "2026-06-27"
draft: false
toc: true
type: docs
featured: true
tags: ["docs", "monitor", "tutorial", "setup", "configuration"]
menu:
  docs:
    parent: Pool Monitor
    name: Users Guide
    weight: 40
---

## Einrichtung

Beim ersten Einschalten (oder nach einem Factory-Reset) startet das Gerät einen
**WLAN-Hotspot** namens `pool-monitor`.

1. Verbinde dich mit deinem Smartphone oder Laptop mit dem WLAN `pool-monitor`
   (offenes Netz, kein Passwort).
2. Ein **Captive Portal** sollte sich automatisch öffnen. Falls nicht, öffne
   einen Browser und gehe zu **`http://192.168.4.1`**.
3. Gib folgende Daten ein:
   - **WLAN-SSID** und **Passwort**
   - **MQTT-Broker-Hostname** oder IP-Adresse
   - **MQTT-Port** (Standard: `1883`)
4. Klicke **Save** — das Gerät startet neu und verbindet sich mit deinem
   Netzwerk.

Nach wenigen Sekunden zeigt das E-Ink-Display die aktuellen Pool-Daten an.

### MQTT-Broker

Der Pool Monitor liest Daten, die der
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller)
über Home-Assistant-MQTT-State-Topics veröffentlicht. Der MQTT-Broker muss
im selben WLAN wie der Monitor erreichbar sein.

---

## Display-Layout

Das E-Ink-Display zeigt folgende Informationen:

```text
┌──────────────────────────────────┐
│ 🌊  Pool-Temperatur     25.3 °C │
│ ☀️  Solar-Temperatur    55.1 °C │
│                                  │
│ ⏱️  Pool-Pumpe          EIN     │
│ ⏱️  Solar-Pumpe         EIN     │
│ ⏱️  Modus               auto    │
│                                  │
│         14:30                    │
│    www.smart-swimmingpool.com    │
└──────────────────────────────────┘
```

- **Temperaturen** werden bei jeder neuen MQTT-Nachricht aktualisiert.
- **Pumpen-Status** zeigt `EIN` / `AUS` basierend auf dem Pool-Controller.
- **Modus** zeigt den aktuellen Betriebsmodus (`auto`, `manual`, etc.).
- **Uhrzeit** wird per NTP (stündlich) mit automatischer Sommerzeit
  synchronisiert.
- **Branding** am unteren Rand zeigt die Projekt-Website.

Das Display aktualisiert sich nur bei Datenänderungen — es bleibt während des
Tiefschlafs unverändert, um Energie zu sparen.

---

## Netzwerkkonfiguration (mDNS)

Das Gerät ist im Netzwerk unter `pool-monitor.local` per mDNS erreichbar.
Dies funktioniert in den meisten Netzwerken ohne zusätzliche Konfiguration.

Der DHCP-Lease bleibt über Deep-Sleep-Zyklen hinweg erhalten — das Gerät gibt
seine IP-Adresse beim Schlafengehen nicht frei. Es bleibt in der Router-Tabelle
sichtbar (wenn auch nicht erreichbar, solange es schläft).

---

## MQTT-Fehler und Konfigurationsportal

Kann der MQTT-Broker nicht erreicht werden (z.B. falscher Hostname, temporärer
Ausfall), startet das Gerät automatisch ein **Konfigurationsportal**:

1. Das Display zeigt:
   - WLAN-SSID (`pool-monitor`)
   - IP-Adresse des Zugangspunkts (`192.168.4.1`)
   - Einen QR-Code für schnellen Zugriff
2. Verbinde dich mit dem WLAN `pool-monitor` (offenes Netz, kein Passwort).
3. Öffne die Konfigurationsseite — entweder über den QR-Code oder unter
   `http://192.168.4.1`.
4. Korrigiere die MQTT-Einstellungen (Hostname, Port) oder WLAN-Zugangsdaten.
5. Klicke **Save** — das Gerät startet neu und verbindet sich erneut.

> **DNS-Tipp**: Wenn der MQTT-Hostname (z.B. `smarthome-pi`) nicht aufgelöst
> werden kann, verwende die IP-Adresse. Der Hostname wird auch ohne DNS
> versucht (PubSubClient löst intern auf), aber eine IP-Adresse ist
> zuverlässiger.

Das Portal bleibt **5 Minuten** aktiv. Wenn in dieser Zeit keine Konfiguration
gespeichert wird, wechselt das Gerät in den Tiefschlaf, um Strom zu sparen. Es
wiederholt den Verbindungsversuch beim nächsten Aufwachen (alle 180 Sekunden).

---

## OTA-Firmware-Updates

Der Pool Monitor unterstützt **Over-the-Air (OTA) Firmware-Updates** über
GitHub Releases:

- Bei jedem Aufwachzyklus prüft das Gerät auf dem GitHub-Repository nach einer
  neueren Firmware-Version.
- Wenn ein Update verfügbar ist, wird die `.bin`-Datei heruntergeladen und
  automatisch geflasht.
- Das Display zeigt kurz "Update" während des Vorgangs.
- Nach dem Update startet das Gerät neu und setzt den Normalbetrieb fort.

Es ist kein manuelles Eingreifen erforderlich — das Update erfolgt vollständig
automatisch während eines regulären Aufwachzyklus. Um eine sofortige Prüfung
auszulösen, genügt ein Aus- und Wiedereinschalten.

---

## Zeitsynchronisation

Das Gerät synchronisiert seine Uhr per NTP (Network Time Protocol) über
`europe.pool.ntp.org`:

- Die **NTP-Synchronisation** erfolgt etwa alle **60 Minuten**.
- Zwischen den Syncs wird die Uhrzeit aus der letzten bekannten Zeit plus der
  vergangenen Betriebszeit rekonstruiert.
- Das Display zeigt immer die **Ortszeit** mit automatischer
  Sommerzeitumstellung (CET/CEST).

Wenn `--:--` auf dem Display angezeigt wird, war die NTP-Synchronisation noch
nicht erfolgreich. Dies klärt sich in der Regel beim nächsten Aufwachen
(innerhalb von 3 Minuten).

---

## Fehlerbehebung

| Symptom | Wahrscheinliche Ursache | Lösung |
|---------|------------------------|--------|
| Display bleibt weiß | Keine oder korrupte Firmware | Firmware per PlatformIO flashen (siehe [Software Guide](software-guide.de.md)) |
| "MQTT Error" auf Display, Portal startet | MQTT-Broker nicht erreichbar | Broker-Hostname/IP prüfen; MQTT-Server läuft? |
| "WiFi connection failed" | Falsche Zugangsdaten oder schwaches Signal | Über Captive Portal neu konfigurieren; WLAN-Reichweite prüfen |
| Display zeigt alte Daten (keine Aktualisierung) | MQTT-Problem oder Pool Controller offline | Prüfen, ob der Pool Controller auf HA-State-Topics veröffentlicht |
| QR-Portal erscheint alle 3 Minuten | MQTT-Einstellungen nicht gespeichert | MQTT-Hostname/IP erneut eingeben; IP statt Hostname versuchen |
| Brownout / Neustart-Schleife | Zu schwaches Netzteil | 5V/≥1A Netzteil verwenden; anderes USB-Kabel probieren |
| Display aktualisiert, zeigt aber `--:--` | NTP-Zeitsynchronisation fehlgeschlagen | Internetverbindung prüfen; NTP wird stündlich versucht |

---

## Referenzen

- [Hardware Guide](hardware-guide.de.md) — Board-Aufbau, Pinbelegung, Solarbetrieb
- [Software Guide](software-guide.de.md) — Entwicklungseinrichtung, Build, MQTT-Topics
- [Pool Controller](https://github.com/smart-swimmingpool/pool-controller)
- [Home Assistant](https://www.home-assistant.io/)
- [Smart Swimming Pool Website](https://www.smart-swimmingpool.com)

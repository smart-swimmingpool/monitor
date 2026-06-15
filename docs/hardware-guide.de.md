---
title: Hardware Guide des Pool Monitors
summary: Schritt-für-Schritt-Hardware-Anleitung für den Pool Monitor — kompatible LILYGO TTGO T5 Boards, Pinbelegung, Stromversorgung und erste Inbetriebnahme
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

## Übersicht

Der Pool Monitor ist ein **fertig aufgebautes Anzeigegerät** auf Basis der
LILYGO TTGO T5 E-Paper-ESP32-Board-Familie. Anders als der
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller)
werden **keine zusätzlichen Sensoren oder Relais benötigt** — nur das Board selbst.

> **Zielgruppe**: Jeder, der mit grundlegender Elektronik vertraut ist (USB-Kabel
> einstecken). Keine Lötkenntnisse erforderlich.
>
> Gesamtkosten: **~35–55€**.

## Sicherheit ⚠️

- Der Pool Monitor arbeitet ausschließlich mit **Kleinspannung (5V USB)**.
- **Allerdings** liest er Daten vom
  [Pool Controller](https://github.com/smart-swimmingpool/pool-controller),
  der **230V Netzspannung** schaltet. Der Monitor empfängt nur MQTT-Daten — er
  kommt **nicht** direkt mit Netzspannung in Berührung.
- Bewahren Sie das Board und das USB-Kabel an einem trockenen Ort auf. Bei
  Außeneinsatz in einem wetterfesten Gehäuse (IP54 oder besser) montieren.

---

## Benötigte Teile (BOM)

| # | Komponente | Menge | ca. Preis | Hinweise |
| --- | --- | --- | --- | --- |
| 1 | LILYGO TTGO T5 V2.3.1 (V231) E-Paper ESP32 Board | 1 | 25–35€ | 2.13" E-Ink Display, integrierter ESP32 |
| 2 | USB-C Kabel (Daten + Strom) | 1 | 3–5€ | Zum Flashen der Firmware und Stromversorgung |
| 3 | USB-Netzteil 5V/≥1A | 1 | 5–10€ | Handelsübliches Handy-Ladegerät |
| 4 | Optional: Gehäuse (ABS/PVC, IP54+) | 1 | 5–10€ | Für Außeneinsatz / Spritzwasserschutz |
| **Gesamt** | — | — | **~35–55€** | Komplettset, kein Löten erforderlich |

### Wo kaufen?

Alle Teile sind bei Amazon, AliExpress, eBay oder bei Elektronik-Distributoren
wie Reichelt, Pollin, Conrad (DE/AT/CH) erhältlich.

- **TTGO T5**: Suche nach "TTGO T5 V2.3.1 e-paper ESP32" oder "LILYGO T5
  e-ink display". Achte auf die **Revision V2.3.1** für das neueste E-Ink-Panel.
- **USB-C Kabel**: Jedes datenfähige Kabel funktioniert. Vermeide
  reine Ladekabel zum Flashen.
- **Netzteil**: Standard-USB-Handyladegerät (5V/≥1A). Ein qualitatives Netzteil
  verhindert Spannungseinbrüche während des WiFi-Betriebs.

---

## Kompatible Board-Varianten

Die TTGO T5 Serie gibt es in mehreren Revisionen. Die Firmware unterstützt:

| Board-Variante | Display-Panel | Status | Hinweise |
|----------------|---------------|--------|----------|
| **T5 V2.3.1 (V231)** | 2.13" GxDEPG0213BN (s/w) | ✅ **Standard** | **Empfohlen** — aktuellste Revision, aktiv getestet |
| T5 V1.2 / V2.4 | 2.13" GxGDE0213B1 (s/w) | ✅ Unterstützt | Älteres Panel, anderer Treiber |
| T5 V2.0 / V2.3 | 2.13" GxGDE0213B1 (s/w) | ✅ Unterstützt | Ohne SD-Kartenslot |
| T5 V2.1 | 2.9" GxGDEH029A1 (s/w) | ✅ Unterstützt | Größeres 2.9" Display |
| T5 V2.2 | 2.9" GxGDEH029A1 (s/w) | ✅ Unterstützt | Andere Display-Pinbelegung |
| T5 V2.8 | 2.7" GxGDEW027W3 (s/w) | ✅ Unterstützt | Mit Audio-DAC |

Um eine andere Variante auszuwählen, `src/board_def.h` editieren und den
entsprechenden Define setzen:

```cpp
#define LILYGO_T5_V231 1   // Standard — für andere Varianten auskommentieren
// #define TTGO_T5_2_1  1  // Beispiel: für 2.9"-Variante aktivieren
```

Dann die Firmware neu bauen (siehe [Software Guide](software-guide.de.md)).

---

## Pinbelegung

Die Pin-Konfiguration ist in `src/board_def.h` definiert und variiert je nach
Board-Variante. Für das Standard-Board **V2.3.1 (V231)**:

| Signal | GPIO | Hinweise |
| --- | --- | --- |
| E-Ink BUSY | GPIO4 | Display-Busy-Ausgang |
| E-Ink RESET | GPIO16 | Display-Reset-Leitung |
| E-Ink DC | GPIO17 | Data/Command-Steuerung |
| E-Ink SS (CS) | GPIO5 | SPI Chip-Select |
| SPI MOSI | GPIO23 | Display-Datenleitung |
| SPI MISO | — | Nicht verbunden (E-Ink ist schreibgeschützt) |
| SPI CLK | GPIO18 | Display-Takt |
| MODEM_POWER_ON | GPIO23 | Modem-Stromversorgung (im Deep Sleep ausgeschaltet) |
| LED_BUILTIN | GPIO2 | Eingebaute Status-LED |

> Die SPI-Pins werden mit dem Flash-Speicher geteilt — das ist normal und
> funktioniert, weil das E-Ink-Display nur bei idle CPU zugegriffen wird.

### Hinweise zur Pinbelegung

- **SPI MISO** wird nicht genutzt, da das E-Ink-Display nur Daten empfängt
  (kein Rücklesen nötig).
- **GPIO23** hat eine Doppelfunktion (SPI MOSI + MODEM_POWER_ON). Im Deep Sleep
  wird es auf LOW geschaltet, um externe Modem-Stromversorgung abzuschalten.
- Die Strapping-Pins (GPIO0, GPIO2, GPIO12, GPIO15) werden für die
  E-Ink-Ansteuerung vermieden, um Boot-Probleme zu verhindern.

---

## Verdrahtung

**Es ist keine manuelle Verdrahtung erforderlich.** Das TTGO T5 Board integriert
alle Komponenten:

- ESP32 Mikrocontroller (Dual-Core Xtensa LX6)
- 2.13" E-Ink Display (SPI-verbunden, GxDEPG0213BN Panel)
- USB-Seriell-Wandler (CP210x oder CH340)
- 3.3V Spannungsregler
- Li-Po Ladeschaltung (optionaler Akku-Anschluss)

Das Board wird einfach per **USB-C** angeschlossen:

- Am **Computer** zum Flashen der Firmware
- An einem **USB-Netzteil** für den Dauerbetrieb

### Taster (bei unterstützten Varianten)

Einige TTGO T5 Varianten haben Hardware-Taster. Die Firmware nutzt sie
derzeit **nicht** — die gesamte Konfiguration erfolgt über das Captive Portal
(siehe [Users Guide](users-guide.de.md)).

| Taster | GPIO | Hinweise |
| --- | --- | --- |
| BUTTON_1 | GPIO37 | Bei T5 V2.4/2.8 Varianten vorhanden |
| BUTTON_2 | GPIO38 | Bei T5 V2.4/2.8 Varianten vorhanden |
| BUTTON_3 | GPIO39 | Bei den meisten Varianten vorhanden |

---

## Stromversorgung

| Quelle | Spannung | Strom | Hinweise |
| --- | --- | --- | --- |
| USB-C Netzteil | 5V | ≥1A | **Empfohlen** für den Dauerbetrieb |
| Computer-USB-Port | 5V | ~500mA | Geeignet zum Flashen und Testen |
| USB-Powerbank | 5V | variiert | Möglich, aber Deep Sleep ist für Netzbetrieb ausgelegt |

### Stromverbrauch

| Zustand | Strom | Dauer pro Zyklus |
| --- | --- | --- |
| Aktiv (WiFi + MQTT + Display-Update) | ~80mA | ~10–15 Sekunden |
| Deep Sleep | ~10µA | ~165–170 Sekunden |
| **Durchschnitt** | **~5.5mA** | über gesamten 180s-Zyklus |

Durch den 180-Sekunden-Deep-Sleep-Zyklus verbringt das Board **>90% der Zeit
im Tiefschlaf**, was es auch für Batteriebetrieb geeignet macht (das TTGO T5
hat einen eingebauten Akku-Anschluss und Ladeschaltung).

---

## Erste Inbetriebnahme

### 1. Sichtprüfung

Vor dem Anschließen:

- USB-C-Anschluss auf Fremdkörper prüfen
- Board nicht auf leitfähiger Oberfläche platzieren
- Auf sichtbare Schäden (Aufblähungen, Korrosion) prüfen

### 2. Einschalten

1. TTGO T5 per USB-C an Stromquelle (Computer oder Netzteil) anschließen.
2. Das E-Ink Display aktualisiert sich nach wenigen Sekunden:
   - **Ohne Firmware**: Testmuster oder leerer Bildschirm.
   - **Mit Pool Monitor Firmware**: Der Startbildschirm — siehe
     [Users Guide](users-guide.de.md#einrichtung) für die Ersteinrichtung.

### 3. Ersteinrichtung

Das Gerät startet im **AP-Modus**, wenn kein WiFi konfiguriert ist:

1. Verbinde dich mit dem WLAN **`pool-monitor`** (offenes Netz, kein Passwort).
2. Ein Captive Portal öffnet sich automatisch — oder öffne
   **`http://192.168.4.1`**.
3. Gib deine WLAN-Zugangsdaten und MQTT-Broker-Einstellungen ein.
4. Klicke **Save** — das Gerät startet neu und verbindet sich mit deinem Netzwerk.

Details im [Users Guide](users-guide.de.md).

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

- [LILYGO TTGO T5 Produktseite](http://www.lilygo.com/prod_view-66.html)
- [GxEPD Bibliothek](https://github.com/ZinggJM/GxEPD)
- [ESP32 Datenblatt](https://www.espressif.com/en/products/socs/esp32)
- [Pool Controller Hardware Guide](https://github.com/smart-swimmingpool/pool-controller/blob/main/docs/hardware-guide.md)
- [Board-Definitionen (Quelle)](https://github.com/smart-swimmingpool/monitor/blob/main/src/board_def.h)

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

## Solarbetrieb ☀️

Der Pool Monitor eignet sich aufgrund seines extrem niedrigen Energieverbrauchs
(Durchschnitt ~5,5 mA bei 5 V) hervorragend für den autonomen Solarbetrieb.
Mit einer kleinen Solarzelle und einem Li‑Po‑Akku kann das Gerät dauerhaft
ohne USB‑Netzteil betrieben werden.

> **Voraussetzung**: Das TTGO T5 Board hat einen **eingebauten Li‑Po‑Lader
> (TP4056)** und einen **JST‑1,25‑mm‑Batteriestecker**. Für den Solarbetrieb wird
> nur ein Solarpanel und ein Akku benötigt – das Board übernimmt die Ladeelektronik.

---

### Leistungsbudget

| Zustand | Strom | Dauer pro Zyklus |
| --- | --- | --- |
| Aktiv (WiFi + MQTT + Display‑Update) | ~80 mA | ~10–15 Sekunden |
| Deep Sleep | ~10 µA | ~165–170 Sekunden |
| **Durchschnitt** | **~5,5 mA** | über 180‑s‑Zyklus |
| **Täglicher Energiebedarf** | **~132 mAh** | bei 5 V (≙ 660 mWh) |

Der tägliche Energiebedarf von **~130 mAh** (bei 5 V) ist selbst mit einer
kleinen Solarzelle an den meisten Standorten deckbar.

---

### Benötigte Teile (zusätzlich)

| # | Komponente | Menge | ca. Preis | Hinweise |
| --- | --- | --- | --- | --- |
| 1 | Solarpanel, 5 V / 1–2 W | 1 | 10–25 € | Monokristallin, offene Klemmenspannung 5,5–6 V |
| 2 | Li‑Po‑Akku, 3,7 V, 1000–2000 mAh | 1 | 8–15 € | Mit JST‑1,25‑mm‑Stecker (2‑polig, roter Draht = Plus) |
| 3 | USB‑C Breakout‑Board oder **XC6206‑5.0V** Reglermodul | 1 | 2–5 € | Zur stabilen 5 V‑Versorgung aus dem Solarpanel |
| 4 | Optional: Schottky‑Diode (1N5819) | 1 | 1 € | Rückflussschutz vom Board zum Panel bei Dunkelheit |
| 5 | Optional: Gehäuse (IP65+, transparente Abdeckung) | 1 | 10–20 € | Solarpanel kann Deckel ersetzen oder integriert werden |
| **Gesamt** | — | — | **~30–65 €** | Zusätzlich zum Basis‑BOM |

#### Bezugsquellen für Solarkomponenten

- **Solarpanel**: Suche nach *„5V 2W Solarpanel“* oder *„6V 200mA Solarmodul“*,
  z. B. bei Reichelt, Pollin, Amazon oder AliExpress. Ideal sind monokristalline
  Panels mit ETFE-Beschichtung für Außeneinsatz.
- **Li‑Po‑Akku**: Suche nach *„3.7V 1000mAh Li‑Po JST 1.25“* oder
  *„LP401230 1000mAh“*. Achte auf den **1,25‑mm‑JST‑Stecker (2‑polig)**.
  Gängige Formate: 401230 (1000 mAh, ca. 30×12×4 mm) oder 503048 (1500 mAh).
- **USB‑C Breakout**: Für direkte 5 V‑Einspeisung über den USB‑C‑Port.

---

### Schaltungsvarianten

#### Variante A: Solarpanel → USB‑C (empfohlen, einfach)

Die einfachste und sicherste Methode – nutzt den **eingebauten TP4056 Lader**
des TTGO T5.

```text
  Solarpanel          TTGO T5 V231
  (5-6V)                  ┌────────┐
     │                    │ USB-C  │
     ├─[USB-C Breakout]───┤  Port  │
     │                    │        │
  Li‑Po Akku              │  JST   │
  (3.7V, 1000mAh) ────────┤  BAT   │
                           └────────┘
```

1. **Solarpanel** (5–6 V) an ein USB‑C Breakout‑Board oder direkt an einen
   USB‑C Stecker löten (VBUS auf Pin A4/A9/B4/B9, GND auf A1/B1).
2. USB‑C Breakout in den USB‑C Port des TTGO T5 stecken.
3. **Li‑Po‑Akku** an den JST‑1,25‑mm‑Stecker (BAT) des TTGO T5 anschließen.

*Hinweis*: Das Panel muss **5 V (nominal) bis max. 6 V** liefern. Der TP4056
benötigt mindestens ~4,5 V zum Starten des Ladevorgangs.

#### Variante B: Externer Solarlader (flexibler)

Verwendet ein separates Solarlader-Modul (z. B. **CN3065** oder **TP4056 mit
Solar-Eingang**) für bessere Energieausbeute bei schwachem Licht.

```text
  Solarpanel          Solarlader           TTGO T5 V231
  (5-6V)              Modul                ┌────────┐
     │                  │                  │ USB-C  │
     ├──────────────────┤ IN+         OUT+─┤  Port  │
     │                  │                  │        │
                         │ BAT+  ────────   │  JST   │
  Li‑Po Akku ───────────┤ BAT-      ──────┤  BAT   │
                         │                  └────────┘
```

1. Solarpanel an den **IN+ / IN−** Anschluss des Ladereglers.
2. Li‑Po‑Akku an **BAT+ / BAT−** des Ladereglers.
3. Lader‑Ausgang an den JST‑BAT‑Stecker des TTGO T5 **oder** über einen
   5 V‑Step‑Up‑Wandler an den USB‑C Port.

> **Vorteil**: Module wie der CN3065 haben einen **MPPT‑ähnlichen
> Eingangsregler**, der auch bei Bewölkung noch Energie liefert.

---

### Minimalkonfiguration (Sommerbetrieb) ☀️

Wenn der Monitor **nur in der warmen Jahreszeit** (Mai–September) und bei
**überwiegend gutem Wetter** laufen soll, reichen minimale Komponenten:

| Komponente | Minimal | Begründung |
| --- | --- | --- |
| **Solarpanel** | **0,3–0,5 W** (5 V, ~60–100 mA) | Im Sommer ~250 mAh/Tag Ertrag – das Doppelte des Bedarfs |
| **Akku** | **500 mAh** (z. B. LP401015) | Überbrückt die Nacht (~66 mAh) + 2 bewölkte Tage |
| **Schottky-Diode** | Optional, aber empfohlen | Verhindert nächtlichen Rückstrom bei kleinem Panel |

Der **tägliche Bedarf von ~130 mAh** wird im Sommer bereits von einem
**0,3‑W‑Panel** (60 mA × 6 h × 0,7 Wirkungsgrad ≈ 250 mAh/Tag) übertroffen.
Die Nacht überbrückt der Akku – selbst nach einem bewölkten Tag ist noch
genug Reserve für den Folgetag.

> **Praxis-Tipp**: Ein Panel unter 0,5 W ist oft kaum günstiger als ein
> 0,5‑W‑Panel. Empfehlung daher: **0,5 W + 500‑mAh‑Akku** als günstigste
> Kombination (Kosten ~15–25 € zusätzlich zum Board).

### Dimensionierung

#### Akku-Größe

| Autonomie | Empfohlene Kapazität | Beispiel |
| --- | --- | --- |
| Nacht + 1 Regentag (Sommer-Minimum) | 500 mAh | LP401015 / 501215 (~15×12×4 mm) |
| 2–3 Tage (Sicherheit) | 1000 mAh | LP401230 (30×12×4 mm) |
| 5–7 Tage (Winter/trüb) | 2000 mAh | LP503048 (50×30×5 mm) |
| 10+ Tage (Notversorgung) | 3000 mAh | LP604560 (60×45×6 mm) |

**Faustregel**: In Deutschland sind im Winter durchschnittlich **2–3 Sonnenstunden
pro Tag** üblich (Nov–Feb). Ein 1000‑mAh‑Akku überbrückt 5–7 Tage ohne Sonne.
Ein 2000‑mAh‑Akku reicht für 10–14 Tage.

#### Solarpanel-Größe

| Panel | Spannung | Strom (max) | Tagesertrag (Winter) | Tagesertrag (Sommer) |
| --- | --- | --- | --- | --- |
| 0,3 W | 5 V | ~60 mA | ~30 mAh | ~150 mAh |
| **0,5 W** | **5 V** | **~100 mA** | **~50 mAh** | **~250 mAh** |
| 1 W | 5 V | ~200 mA | ~100 mAh | ~500 mAh |
| 2 W | 5 V | ~400 mA | ~200 mAh | ~1000 mAh |

**Empfehlung**:

- **Nur Sommer**: **0,5 W + 500‑mAh‑Akku** – klein, leicht, günstig
- **Ganzjahresbetrieb (DE/AT/CH)**: **1–2 W + 1000–2000‑mAh‑Akku**

#### Panel‑Abmessungen (Richtwerte)

| Leistung | Typische Maße (Breite × Höhe) | Dicke |
| --- | --- | --- |
| 0,5 W | 90 × 60 mm | 2–3 mm |
| 1 W | 110 × 70 mm | 2–3 mm |
| 2 W | 155 × 85 mm | 2–3 mm |
| 3 W | 185 × 95 mm | 2–3 mm |

---

### Verdrahtung – Schritt für Schritt

#### Vorbereitung

1. **TTGO T5 vom Strom trennen** (USB‑C und Akku entfernen).
2. Alle Komponenten auf einer nicht-leitenden Unterlage auslegen.
3. Bei Verwendung von Litzen: Adern verzinnen, Schrumpfschlauch vorbereiten.

#### Variante A (empfohlen) – Schritt-für-Schritt

1. **USB‑C Breakout vorbereiten**: Die Pads VBUS und GND mit einem
   feinen Lötkolben verzinnen.
2. **Solarpanel vorbereiten**: Die Litzen des Panels mit Schrumpfschlauch
   isolieren; Plus (rot) an VBUS, Minus (schwarz) an GND des Breakouts.
3. **Optional: Schottky‑Diode 1N5819** in Reihe zur Plus‑Leitung setzen
   (Kathode zum Breakout, Anode zum Panel), um Rückstrom bei Dunkelheit
   zu verhindern.
4. **Breakout einstecken**: USB‑C Breakout vorsichtig in den USB‑C Port
   des TTGO T5 stecken.
5. **Li‑Po‑Akku anschließen**: Den Akku‑Stecker in die **JST‑1,25‑mm‑Buchse
   (BAT)** auf dem TTGO T5 stecken. Achtung: Verpolung – roter Draht = Plus.
6. **Sichtprüfung**: Keine losen Drähte, keine Kurzschlüsse.

#### Test

1. Nur **Solarpanel** anschließen (ohne Akku) – die rote LED auf dem TTGO T5
   sollte leuchten, wenn das Panel ausreichend Licht bekommt. Das Board sollte
   booten.
2. Panel abstecken, **Akku allein** anschließen – das Board sollte aus dem Akku
   booten.
3. **Beide anschließen** – Normalbetrieb. Der TP4056 lädt den Akku bei
   Lichteinfall und schaltet bei Dunkelheit automatisch auf Akku um.
4. Firmware testen: WiFi‑Verbindung, MQTT, Display‑Update.

---

### Gehäuse und Aufstellung

- **Wetterfestes Gehäuse (IP65+)** verwenden, z. B. ABS‑Kunststoffgehäuse von
  Bopla, Spelsberg oder Hammond.
- Das **Solarpanel** kann entweder:
  - **außen auf dem Gehäuse** montiert werden (Kabel durch Dichteinführung),
  - oder bei transparentem Deckel **innen hinter einer klaren Scheibe** liegen
    (Ertrag dann ca. 10–20 % geringer).
- **Aufstellort**: Südausrichtung, Neigung 30–45°, kein Schattenwurf
  (Bäume/Gebäude). In DE/AT/CH reicht bereits eine **Mittagssonne von 2–3 h**
  für den Minimalbetrieb.
- **Lüftungsschlitze** sind wegen der geringen Verlustleistung nicht nötig.
- **Temperaturen**: Der Li‑Po‑Akku sollte nicht über 60 °C erwärmt werden.
  Bei direkter Sonneneinstrahlung auf das schwarze Gehäuse Hitzestau vermeiden
  (ggf. helles Gehäuse oder Schattenplatz).

---

### Hinweise und Sicherheit ⚠️

- **Li‑Po‑Sicherheit**: Nur Akkus mit **Überlade‑/Tiefentladeschutz (PCB)**
  verwenden. Bei Beschädigung (Beule, Aufblähung) sofort austauschen.
- **Niemals** das Solarpanel an die **JST‑BAT‑Buchse** anschließen – dort gehört
  nur ein **Akku** hin. Die Eingangsspannung des TP4056 ist auf max. 6 V
  begrenzt; höhere Spannung zerstört den Charger.
- **USB‑C Datenleitung**: Bei Variante A wird der USB‑C Port belegt.
  Zum Flashen der Firmware muss das Breakout abgezogen werden – es sei denn,
  man nutzt **UART über die Stiftleisten** (RX/TX) am Board.
- **Kabelzugentlastung**: Im Gehäuse Kabelbinder oder Kleberpunkte gegen
  Zugbelastung auf den Steckern vorsehen.

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

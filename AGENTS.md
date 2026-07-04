# AGENTS.md

Zweck: Regeln und Standards für einen Coding-Agent, der PlatformIO-basierten IoT-Firmware für ESP32-basierten E-Ink Pool Monitor entwickelt, refaktoriert, analysiert und stabilisiert. Fokus: Architektur, Linting, Speicher- und Ressourcenmanagement, Sicherheit, Deep-Sleep-Zyklen, Wartbarkeit, Tests und Updates.

## 1. Arbeitsweise

Der Agent:

- liefert minimal-invasive Änderungen mit Begründung, Impact-Analyse (RAM/Flash/CPU), Risiken, Rollback-Hinweisen.
- berücksichtigt Deep-Sleep-Betrieb: ESP32 wacht alle 180 Sekunden auf, verliert dabei alle RAM-States zwischen Zyklen.
- verhindert blockierende Patterns im Laufzeit-Code (long `delay()`, busy waits, blockierende Netzwerk-Calls).
- vermeidet unsichere dynamische Allokationen und ungeprüfte Heap-Nutzung.
- folgt einem CI-regelbasierten Prozess (Build + Tests + Lint).

## 1b. Empfohlene Skills für dieses Repo

### Hermes-Skills (externe Skills per Skill-Tool laden)

Für größere oder riskantere Änderungen in diesem Repository sollten diese Skills geladen werden:

- `monitor-migration-workflow` — projektbezogener Umbrella-Workflow für Home-Assistant-/CI-Migrationen.
- `writing-plans` — wenn eine Änderung in mehrere kleine, überprüfbare Schritte zerlegt werden soll.
- `test-driven-development` — wenn Firmware- oder Parser-Logik geändert wird.
- `systematic-debugging` — wenn Build-, MQTT- oder Laufzeitfehler untersucht werden.
- `requesting-code-review` — vor Commit/Push für Qualitäts- und Sicherheitschecks.
- `subagent-driven-development` — für größere, klar trennbare Aufgaben.
- `local-service-update-workflow` — wenn PlatformIO/CI/Update-Checks angepasst werden.

### Projekt-Skill: Clean Code

Dieser Skill ist **lokal im Projekt** unter `.opencode/skills/clean-code/SKILL.md` und muss vor/nach jeder Code-Änderung geladen werden:

- `clean-code` — Clean-Code-Review und Quality-Gate für ESP32/Arduino-Firmware. Enthält 10 prüfbare Regeln (toter Code, DRY, Namenskonventionen, constexpr, Magic Numbers, Single Responsibility, Embedded Safety, Busy-Waits, Singletons, Include-Was-You-Use) sowie eine automatisierte Check-Suite.

**Wann laden:**
- ✅ Vor jedem Commit/PR
- ✅ Bei Code-Review
- ✅ Nach Feature-Implementierung
- ✅ Bei Bug-Fix mit Refactoring-Potential
- ❌ Nicht bei reinen Dokumentations- oder Konfig-Änderungen

## 2. Zielplattformen & Framework

- Plattform: ESP32 (LILYGO T5 V231 mit E-Ink Display); Framework: Arduino.
- PlatformIO: Single-Source für Build-Konfiguration (`platformio.ini`), Projekt-Environments, Lib-Pins.
- Wichtige Abhängigkeiten: GxEPD (E-Ink), PubSubClient3 (MQTT), WiFiSettings (Configuration), NTPClient (Time Sync), Home Assistant MQTT Discovery im Zielsystem.

## 3. Projektstruktur und Architektur

Aktuelle Struktur:

- `src/` Hauptcode (main.cpp, Display-Treiber, NTP-Logik)
- `src/GxDEPG0213BN/`, `src/GxGDE0213B72B/` Hardware-spezifische E-Ink Treiber
- `docs/` Dokumentation (User Guide, Hardware Guide, Software Guide)
- `lib/` Externe Libraries
- `platformio.ini` Build-Konfiguration

Architekturregeln:

- Deep-Sleep-Bewusstsein: Alle States zwischen Zyklen in Preferences (NVS) persistieren.
- E-Ink Display: Sparsame Updates (hoher Energieverbrauch), nur bei Datenänderungen aktualisieren.
- MQTT-Client: Verbinden, Daten empfangen, Display aktualisieren, Deep Sleep - kein dauerhafter Betrieb.
- NTP-Zeit-Synchronisation: Nur alle 3600 Sekunden, nicht bei jedem Wake-up, mit Fallback auf gespeicherte Zeit.
- Fehler-Resilienz: Netzwerkfehler tolerieren, graceful Degradation bei fehlenden Daten.

## 4. Coding-Standards

- Sprache: C++ (Arduino-kompatibel), keine unkontrollierten Exceptions.
- Buffer Safety: **Immer `snprintf()` statt `sprintf()` verwenden** zur Vermeidung von Buffer Overflows.
- Header: `include-what-you-use`, kein globales `using namespace`.
- Konstanten: `constexpr`, `const`.
- Ressourcenmanagement: **Immer `preferences.end()` vor `ESP.restart()` oder Deep Sleep aufrufen**.
- Fehlerbehandlung immer explizit, kein stilles Ignorieren.
- Logging: Seriell mit verständlichen Emojis (🖥️, 📡, ⏰, etc.) zur besseren Lesbarkeit.

## 5. Linting & Format

- **Static Analysis**: `platformio check --environment LILYGO_T5_V231 --skip-packages` (Primary Quality Check, CI-pflichtig).
- **Lint**: `cppcheck` oder vergleichbare Tools (optional, nicht CI-pflichtig).
- **Duplicate Code Detection**: JSCPD mit 30% Threshold, exclude `docs/` und `Gx*/` Verzeichnisse.
- **CI**: Jeder Push/PR durchläuft `platformio check` + Build (`pio run`) in `.github/workflows/plaform.io.yml`.
- **Quality-Check-Gate**: Vor jedem Commit/PR immer zuerst die lokalen Quality Checks ausführen und erkannte Findings direkt beheben, statt sie erst in CI zu entdecken.
- **Super-Linter-Parität**:
  - `CPPLINT.cfg` bleibt im Repository-Root; nicht nach `.github/linters/` verschieben.
  - Markdown-Dateien müssen ohne Trailing Spaces, mit genau einer H1 pro Datei und ohne nackte URLs gepflegt werden.
  - Workflow- und Konfigurationsdateien (`*.yml`, `*.yaml`, `*.json`) müssen lokal auf Syntax geprüft werden, wenn sie geändert werden.
  - Für lokale Runs werden `cpplint` und `yamllint` per `pip` installiert; `markdownlint-cli2` und `jscpd` können per `npx` direkt ausgeführt werden.
- **Lokal ausführen**: Änderungen zuerst stagen; die folgende Bash-only Sequenz nutzt `mapfile`, Bash-Arrays und `git diff --cached`.

  ```bash
  platformio check --environment LILYGO_T5_V231 --skip-packages
  platformio run --environment LILYGO_T5_V231
  mapfile -t cpp_files < <(git diff --cached --name-only -- '*.cpp' '*.hpp' '*.h')
  ((${#cpp_files[@]})) && cpplint "${cpp_files[@]}"
  mapfile -t md_files < <(git diff --cached --name-only -- '*.md')
  ((${#md_files[@]})) && npx --yes markdownlint-cli2 "${md_files[@]}"
  mapfile -t yaml_files < <(git diff --cached --name-only -- '*.yml' '*.yaml')
  ((${#yaml_files[@]})) && yamllint "${yaml_files[@]}"
  npx --yes jscpd --config .jscpd.json src .github/workflows
  python -m json.tool .jscpd.json > /dev/null
  ```

## 6. Commit-Konventionen (Conventional Commits)

Commit Messages müssen dem Conventional Commits-Standard folgen: `<type>[optional scope]: <description>`
Commit Types umfassen mindestens:

- `feat` für neue Funktionen
- `fix` für Fehlerbehebungen
- `docs` für Änderungen an Dokumentation
- `style` für Formatierung/Code Style
- `refactor` für Code-Umstrukturierungen ohne funktionale Änderung
- `perf` für Performance-Optimierungen
- `test` für Test-Änderungen
- `chore` für Wartung/Tooling/Build-Änderungen

Commit Messages müssen dem Format entsprechen, damit automatische Changelog-Generierung, Versionierung und CI-Checks funktionieren.

## 7. Build-Konfiguration

- `platformio.ini`: zentrale Flags, Board-Definitionen (`LILYGO_T5_V231`).
- Build-Artefakte: Debug vs Release:
  - Debug: intensiveres Logging, Serial Monitor Output.
  - Release: optimiert, reduziertes Logging für Energieeffizienz.
- Build-Flags: `-D SERIAL_SPEED=115200`, `-D LILYGO_T5_V231`.

## 8. Speicher & Ressourcen (Deep-Sleep-spezifisch)

- **RTC Memory**: Nicht verwendet, da auf ESP32 nicht im Deep Sleep erhalten.
- **NVS (Preferences)**: Für persistente Daten zwischen Deep-Sleep-Zyklen:
  - `last_ntp_sync`: Zeitpunkt der letzten NTP-Synchronisation
  - `total_uptime`: Kumulierte Betriebszeit über alle Sleep-Zyklen
  - `last_epoch`: Letzte bekannte Epoch-Zeit für Zeitrekonstruktion
- **Uptime-Tracking**: `total_uptime` beim Boot inkrementieren (um `TIME_TO_SLEEP_SECONDS`), nicht vor Sleep.
- **Zeit-Rekonstruktion**: Bei übersprungener NTP-Sync: `last_epoch + (total_uptime seit letzter Sync)` verwenden.
- Heap-Management:
  - Keine häufigen dynamischen Allokationen.
  - `StaticJsonDocument` für JSON-Parsing wo möglich.
  - String-Objekte sparsam einsetzen.
- Heap-Metriken überwachen (`ESP.getFreeHeap()`).

## 9. Deep-Sleep-Betrieb

- **Sleep-Zyklus**: 180 Sekunden (`TIME_TO_SLEEP_SECONDS`).
- **Wake-up-Sequenz**:
  1. Preferences laden (NVS)
  2. Uptime inkrementieren
  3. Prüfen ob NTP-Sync nötig (`isNtpSyncNeeded()`)
  4. WiFi verbinden (nur wenn nötig)
  5. MQTT verbinden und Daten empfangen
  6. Display aktualisieren (nur bei Änderungen)
  7. Preferences speichern und schließen
  8. Deep Sleep aktivieren
- **Power-Down**: RTC Memory wird abgeschaltet, alle RAM-Daten gehen verloren.
- **Energieeffizienz**: Minimale Wake-Zeit, WiFi nur wenn nötig, E-Ink nur bei Änderungen.

## 10. NTP & Zeit-Synchronisation

- **NTP-Retry-Pattern**: `forceUpdate()` mit Delay vor Retry, nicht `update()` + `forceUpdate()` in derselben Iteration.
- **Sync-Interval**: Alle 3600 Sekunden (1 Stunde), nicht bei jedem Wake-up.
- **Zeit-Rekonstruktion**: Wenn NTP-Sync übersprungen, Zeit aus `last_epoch + (total_uptime - last_ntp_sync)` berechnen.
- **NTP-Server**: europe.pool.ntp.org
- **Timezone**: CET/CEST mit automatischer Sommerzeit-Umstellung.

## 11. MQTT & Kommunikation

- **Home Assistant MQTT Topics**: Der Pool Monitor soll die retained State-Topics des Pool-Controllers konsumieren statt Homie-Discovery zu verwenden.
- **Subscribed Topics**: Empfangen von Pool-Controller-Daten (Temperatur, Pump-Status, Solar-Status, Betriebsmodus) direkt über HA-Topics.
- **Connection Pattern**:
  1. Verbinden
  2. Subscribe to 5 fixed HA state topics
  3. Warten auf Nachrichten (mit Timeout)
  4. Disconnect vor Sleep
- **Offline-Betrieb**: Graceful degradation bei fehlender MQTT-Verbindung, alte Daten anzeigen.
- **Buffer-Größen**: snprintf-Puffer für Formatierung (50-64 Bytes).

## 12. E-Ink Display

- **Update-Strategie**: Nur bei Datenänderungen, nicht bei jedem Wake-up.
- **Partial vs. Full Updates**: Full Refresh bevorzugt für saubere Darstellung.
- **Display-Inhalt**:
  - Pool-Temperatur
  - Solar-Temperatur
  - Pump-Status (on/off)
  - Solar-Status (on/off)
  - Timestamp
  - Branding (<www.smart-swimmingpool.com>)
- **Icons**: U8g2-Fonts für Icons (Pool, Solar Panel).
- **Energieverbrauch**: Display-Updates sind energieintensiv, sparsam einsetzen.

## 13. Sicherheit

- **Keine Secrets im Repository**: WiFi-Credentials, MQTT-Server über WiFiSettings konfigurierbar.
- **Buffer Safety**: `snprintf()` statt `sprintf()` für alle String-Formatierungen.
- **Resource Management**: `preferences.end()` vor Restart/Sleep zur Vermeidung von Datenverlust.
- **Input Validation**: MQTT-Nachrichten validieren vor Verarbeitung.

## 14. Robustheit & Fehlertoleranz

- **Netzwerk-Resilienz**: WiFi-Reconnect mit Timeout, fortfahren auch bei Verbindungsproblemen.
- **MQTT-Fehler**: Tolerieren und mit letzten bekannten Werten weiterarbeiten.
- **NTP-Fehler**: Retry mit Delay, Fallback auf Zeit-Rekonstruktion.
- **Display-Fehler**: Logging aber kein Absturz.
- **Preferences-Fehler**: Prüfen ob `begin()` erfolgreich, Error-Handling.

## 15. Konfiguration

- **WiFiSettings**: Captive Portal für WiFi und MQTT-Konfiguration.
- **Hotspot-Modus**: Bei fehlender WiFi-Konfiguration automatisch aktiviert.
- **Persistente Config**: In NVS gespeichert.
- **Defaults**: Sinnvolle Standardwerte in Code definiert.

## 16. Tests

- **Keine automatisierten Tests aktuell vorhanden**.
- Zukünftige Tests:
  - Unit-Tests für Zeit-Rekonstruktion
  - Unit-Tests für NTP-Sync-Logik
  - Komponententests für MQTT-Handling
  - Native Tests (`platform = native`) für CI.

## 17. Dependencies

- Minimiert, begründet, Version-Pinned in `platformio.ini`.
- Wichtige Dependencies:
  - GxEPD @ ^3.1.1 (E-Ink Display)
  - PubSubClient3 @ ^3.1.0 (MQTT)
  - WiFiSettings @ ^3.9.2 (Configuration)
  - NTPClient @ ^3.2.1 (Time Sync)
  - U8g2 @ ^2.35.30 (Icons/Fonts)
- Lizenz-Checks; Updates mit CI-Absicherung.

## 18. Release & CI

- **Branch**: `main` (nicht `master`).
- **Build-System**: PlatformIO (nicht Arduino IDE).
- Release: Optimiert, reduziertes Logging.
- CI: Build, Lint (JSCPD), Format-Checks; PlatformIO-Check + Build in CI.
- **CI-System**: GitHub Actions, konfiguriert in `.github/workflows/plaform.io.yml`, `.github/workflows/linter.yml` und `.github/workflows/codeql-analysis.yml`.

## 19. Antipatterns (verboten)

- Unlimitierte `delay()`, Busy-Wait, blockierende Netzwerk-Calls.
- Häufige Heap-Allokationen in kritischen Pfaden.
- `sprintf()` statt `snprintf()` (Buffer Overflow Risiko).
- Vergessen von `preferences.end()` vor Sleep/Restart.
- NTP-Sync bei jedem Wake-up (Energieverschwendung).
- E-Ink Updates ohne Datenänderung (Energieverschwendung).
- RAM-State-Annahmen über Deep-Sleep-Zyklen hinweg.
- `update() + forceUpdate()` in derselben NTP-Retry-Iteration (verursacht unnötigen Netzwerk-Overhead).

## 20. Best Practices aus der Praxis

- **Uptime-Tracking**: `total_uptime` bei Boot um `TIME_TO_SLEEP_SECONDS` inkrementieren, nicht vor Sleep.
- **Zeit-Rekonstruktion**: `last_epoch + elapsed uptime` wenn NTP übersprungen.
- **NTP-Retry**: `forceUpdate()` mit Delay vor Retry.
- **Buffer Safety**: Konsequent `snprintf()` verwenden.
- **Resource Cleanup**: `preferences.end()` vor jedem Neustart/Sleep.
- **Deep-Sleep-Awareness**: Alle wichtigen States in NVS persistieren.
- **Energie-Effizienz**: Minimale Wake-Zeit, WiFi/Display nur wenn nötig.
- **Logging**: Strukturiert mit Emojis für bessere Lesbarkeit im Serial Monitor.

## 21. Projektspezifisches

- **Hauptzweck**: E-Ink Display zur Anzeige von Pool-Daten (Temperatur, Status) aus Home Assistant MQTT Discovery.
- **Energiequelle**: Aktuell USB/Batterie, geplant: Solar-Betrieb.
- **Outdoor-Einsatz**: Geplant in wetterfestem Gehäuse am Pool.
- **Smart Swimmingpool Ecosystem**: Teil des Smart-Swimmingpool-Projekts, kommuniziert mit Pool-Controller.
- **Home-Assistant-kompatibel**: Zielintegration ist Home Assistant MQTT Discovery; Homie ist nur noch Altlast.
- **Dokumentation**: User Guide, Hardware Guide, Software Guide in `docs/`.

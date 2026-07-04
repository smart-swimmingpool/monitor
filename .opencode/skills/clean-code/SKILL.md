---
name: clean-code
description: Project-specific Clean Code review and enforcement for the Pool Monitor firmware (ESP32, PlatformIO, Arduino). Use when reviewing code, planning refactors, or checking quality before commits.
---

# Pool Monitor — Clean Code Skill

## Überblick

Dieser Skill definiert Clean-Code-Standards speziell für das Pool-Monitor-Firmware-Projekt. Er ergänzt die `AGENTS.md` um konkrete, prüfbare Code-Qualitätsregeln für die ESP32/Arduino-Umgebung.

Ziel: Jeder Commit soll die Codebasis lesbarer, wartbarer und robuster machen — nicht nur funktional korrekt.

## Wann verwenden

- **Vor jedem Commit/PR** — als Quality-Gate vor `git push`
- **Bei Code-Review** — als Checkliste für Review-Kommentare
- **Nach Feature-Implementierungen** — um sicherzustellen, dass neue Features sauber eingebaut sind
- **Bei Refactoring** — um zu entscheiden, was verbessert werden soll
- **Bei Bug-Fixes** — um Root Causes nachhaltig zu beheben

## Die 10 Clean-Code-Regeln für dieses Projekt

### R1: Kein toter Code

Toter Code wird **immer entfernt**, nicht auskommentiert.

**Checker:**
```bash
# Unused functions (cppcheck)
pio check --environment LILYGO_T5_V231 --skip-packages

# Unused variables
grep -rn "static.*= nullptr" src/ | grep -v "\.pio"
```

**Projekt-Beispiele aus dem letzten Review:**
- ❌ `NetworkManager::begin()` — nie aufgerufen, aber 34 Zeilen lang
- ❌ `static PoolMonitorContext *Self` — nie gelesen, nur geschrieben
- ❌ `apModeActive_`, `mdnsRunning_`, `hostname_` — nie gelesen

### R2: DRY — Keine Wiederholungen

Wiederholter Code muss in Funktionen oder Makros extrahiert werden.

**Erlaubte Toleranz:** JSCPD Threshold 30% (via `.jscpd.json`)

**Projekt-Beispiele:**
- ❌ Grad-Symbol (3× `drawCircle`) wurde 6× wiederholt → in `drawDegreeSymbol()` extrahiert
- ❌ Pumpen-Icon-Logik 2× → in `drawPumpIcon()` extrahiert

### R3: Konsistenter Namensstil

| Kategorie | Stil | Beispiel |
|-----------|------|----------|
| Klassen | PascalCase | `DisplayManager` |
| Methoden | camelCase | `updateDisplay()` |
| Variablen (local) | camelCase | `bootCount` |
| Member (class) | trailing_ | `mqttCallback_` |
| Member (static) | trailing_ | `prefs_` |
| Konstanten | k-PascalCase | `kCheckIntervalUptime` |
| Datei-Scope | k-PascalCase oder _-suffix | `kHaTopicPoolTemp`, `preferences_` |
| Namespace | PascalCase | `PoolMonitor` |

**Vermeiden:**
- ❌ Makro-artige `HA_TOPIC_POOL_TEMP` als mutable Variable → `constexpr const char* kHaTopicPoolTemp`
- ❌ Gemischte Funktionsstile in derselben Klasse (teils `void func()`, teils `auto func() -> void`)

### R4: constexpr > const > Makro

- Konstanten müssen `constexpr` sein, nicht `#define` oder mutable `const`
- Ausnahme: Build-Flags aus `platformio.ini` (die bleiben `-D`)
- Ausnahme: PROGMEM-Daten

```cpp
// ✅ Richtig
static constexpr uint32_t NIGHT_START_HOUR{22};

// ❌ Falsch
#define NIGHT_START_HOUR 22
static const uint32_t NIGHT_START_HOUR = 22;
const char* HA_TOPIC_TEMP = "...";  // mutable!
```

### R5: Keine Magic Numbers — Benannte Konstanten

Alle Zahlen, die kein offensichtlicher 0/1/nullptr sind, müssen benannt sein.

```cpp
// ✅ Richtig
constexpr int16_t kLayoutSolarY = 94;
displayText("Solar:", kLayoutSolarY, GxEPD_ALIGN_LEFT);

// ❌ Falsch
displayText("Solar:", 94, GxEPD_ALIGN_LEFT);
```

**Besonders wichtig bei:** Pixel-Positionen im Display, Timeouts, Buffer-Größen, GPIO-Pins.

**Ausnahme:** Kurze, lokal-offensichtliche Werte wie `for (int i = 0; i < 3; i++)`.

### R6: Single Responsibility — Keine Monolithen

Keine Funktion/Methode sollte >80 Zeilen haben. KEINE Ausnahme für `setup()`.

**Signale für zu große Funktionen:**
- Sie macht Dinge, die im Namen nicht stehen
- Sie hat Kommentare wie `// ── Phase 2 ──` oder `// Step 1:`
- Sie arbeitet auf 3+ verschiedenen Abstraktionsebenen
- Sie hat mehr als 2 Verschachtelungsebenen

**Projekt-Beispiel (vorher):**
- `PoolMonitorContext::setup()` — 130+ Zeilen, 5+ Verantwortungen (Boot-Erkennung, NVS, Display, Netzwerk, MQTT, OTA)
- → In `setup()` sollte nur noch orchestriert, nicht implementiert werden.

### R7: Embedded-sichere Speichernutzung

Auf dem ESP32 (320KB RAM) gelten besondere Regeln:

- ❌ **Kein** `new` ohne nullptr-Check
- ❌ **Kein** `sprintf()` — immer `snprintf()` mit Größenangabe
- ❌ **Keine** unnötigen `String`-Objekte auf dem Heap (besonders in Callbacks)
- ❌ **Kein** `preferences.end()` vergessen vor `ESP.restart()` oder Deep Sleep
- ❌ **Kein** dynamisches Allozieren in `loop()` oder `setup()` (außer einmalig)

```cpp
// ✅ Richtig
char buf[64];
snprintf(buf, sizeof(buf), "Temp: %.1f°C", temp);

// ❌ Falsch
String msg = "Temp: " + String(temp) + "°C";
```

### R8: Keine Busy-Waits / Blockierende Loops

Blockierende Muster vermeiden — besonders auf dem ESP32 mit Watchdog.

```cpp
// ❌ Falsch
while (true) { delay(1000); }

// ❌ Falsch — bevorzugt Timeout mit Retry-Logik
while (millis() - start < 500) { delay(10); }
```

**Erlaubt:**
- Kurze Delays für Hardware-Settling-Zeiten (< 100ms mit Kommentar)
- `delay(1000)` vor `ESP.restart()` (letzte Aktion)
- `delay(10)` in MQTT-Polling mit Timeout (wie in `initializeMqtt()`)

### R9: Keine globalen Singletons — Instanzen bevorzugen

Alle Manager-Klassen sollten instanzbasiert sein, nicht statisch.

```cpp
// ❌ Falsch — aktueller Zustand
DisplayManager::updateDisplay(temp, ...);

// ✅ Richtig (Ziel)
displayManager.updateDisplay(temp, ...);
```

**Aktuelle Problem-Klassen (für zukünftiges Refactoring):**
- `DisplayManager` — alle Methoden static
- `NetworkManager` — alle Methoden static
- `SystemMonitor` — alle Methoden static, dazu fast komplett im Header
- `OtaUpdater` — alle Methoden static

### R10: Include-Was-You-Use — Keine transitiven Includes

Jede `.cpp`/`.hpp`-Datei inkludiert nur das, was sie direkt benötigt.

```cpp
// ✅ Richtig
#include <functional>  // für std::function<>
#include "Config.hpp"  // für PIN_*

// ❌ Falsch — IPAddress wird im Header nicht verwendet
#include <IPAddress.h>

// ❌ Falsch — ESPmDNS wird nur in toter Funktion verwendet
#include <ESPmDNS.h>
```

**Projekt-Beispiele aus dem letzten Review:**
- `NetworkManager.hpp` inkludierte `<IPAddress.h>` — nie im Header verwendet
- `NetworkManager.cpp` inkludierte `<ESPmDNS.h>` — nur in toter `begin()`-Funktion

## Review-Checkliste

Vor jedem Commit/PR jede Zeile abhaken:

- [ ] **R1:** Kein toter Code (unused functions/variables via `pio check`)
- [ ] **R2:** Keine offensichtlichen Wiederholungen (via JSCPD)
- [ ] **R3:** Namenskonventionen eingehalten
- [ ] **R4:** Konstanten sind `constexpr` (nicht `#define` oder mutable)
- [ ] **R5:** Keine Magic Numbers (außer 0/1/nullptr)
- [ ] **R6:** Keine Funktion >80 Zeilen
- [ ] **R7:** Embedded-sichere Speichernutzung (snprintf, keine Heap-Allokation, preferences.end())
- [ ] **R8:** Keine blockierenden Busy-Waits
- [ ] **R9:** Keine neuen statischen Singleton-Klassen
- [ ] **R10:** Include-Was-You-Use

## Automation

Dieser Skill kann automatisch prüfen mit:

```bash
# Vollständiger Clean-Code-Check (aus Projekt-Root)
echo "=== R1: Dead Code (pio check) ==="
pio check --environment LILYGO_T5_V231 --skip-packages

echo "=== R2: Duplications (jscpd) ==="
npx --yes jscpd --config .jscpd.json src .github/workflows

echo "=== R4: #define statt constexpr ==="
grep -rn "#define" src/ | grep -v "__" | grep -v "_H" | grep -v "FW_VERSION" | grep -v "GITHUB_REPO" | grep -v "SERIAL_SPEED" || echo "OK"

echo "=== R5: Magic Numbers (Verdachtsfälle) ==="
grep -rnP '(?<!\w)(?:[1-9]\d{2,}|0x[0-9a-fA-F]{2,})(?!\w)' src/*.cpp src/*.h src/PoolMonitor/*.cpp src/PoolMonitor/*.hpp | \
  grep -v "drawGlyph\|drawCircle\|constexpr\|sizeof\|GPIO\|PIN_\|width()\|height()\|millis()\|ELINK_\|SPI_\|MODEM_\|LED_\|NIGHT_\|TIMEOUT\|SLEEP\|MQTT_\|BOOT_\|kCheckInterval\|kRetryInterval\|kDownloadTimeout\|kOtaBuffer\|MDNS\|ESP_RST" || echo "Keine Magic Numbers gefunden"

echo "=== R6: Function Length ==="
# Prüft, ob es Funktionen > 80 Zeilen gibt
awk '/^(auto |bool |void |int |uint|float |String |static |static auto |static bool )/ && /\(/ { if (NR > 1) { if (line_count > 80) printf "%s:%d: %d Zeilen\n", file, start_line, line_count } line_count = 0; start_line = NR; file=FILENAME } { line_count++ } END { if (line_count > 80) printf "%s:%d: %d Zeilen\n", file, start_line, line_count }' src/PoolMonitor/*.cpp

echo "=== R7: sprintf ==="
grep -rn "sprintf(" src/ || echo "OK (kein sprintf gefunden)"

echo "=== R10: Include-Check ==="
grep -rn "#include <ESPmDNS.h>\|#include <IPAddress.h>" src/PoolMonitor/NetworkManager* || echo "OK"
```

## Verweise

- **AGENTS.md** (Projekt-Root) — übergeordnete Projektregeln (Deep-Sleep, PlatformIO, CI)
- **CPPLINT.cfg** (Projekt-Root) — Google C++ Style Guide-Varianten
- **`.jscpd.json`** (Projekt-Root) — Duplicate-Code-Threshold
- **`platformio.ini`** (Projekt-Root) — Build-Config und Library-Versionen

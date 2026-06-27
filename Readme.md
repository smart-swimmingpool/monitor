# Pool Monitor | Smart Swimmingpool

[![Smart Swimmingpool](https://img.shields.io/badge/%F0%9F%8F%8A%20-Smart%20Swimmingpool-blue.svg)](https://github.com/smart-swimmingpool)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[![works with Home Assistant](https://img.shields.io/badge/works%20with-Home%20Assistant-41BDF5.svg?logo=home-assistant&logoColor=white "works with Home Assistant")](https://www.home-assistant.io/)

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/J3J33A8DT)

The _Pool Monitor_ is a compact E-Ink display device that shows current pool
data by subscribing to Home Assistant MQTT topics from the
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller):

* Pool water temperature
* Solar storage temperature
* Pool pump status (on/off)
* Solar heating status (on/off)

All data is read-only — the monitor displays what the pool controller publishes.
No sensors, relays, or wiring required. Just power via USB-C and configure
WiFi + MQTT through the captive portal.

## Features

* [x] WiFi configuration via captive portal (ESP-WiFiSettings)
* [x] MQTT connection to existing pool controller
* [x] Reads Home Assistant MQTT state topics
* [x] NTP time synchronisation with automatic CET/CEST
* [x] Deep-sleep operation (180 s cycle, ~5.5 mA average)
* [x] Solar-powered operation (documented in [Hardware Guide](docs/hardware-guide.md))
* [x] OTA firmware updates via GitHub Releases
* [x] System monitoring with watchdog and memory checks
* [x] Boot-loop detection for reliable operation
* [x] Configuration portal with QR code on MQTT errors

## Planned Features

* [ ] Support for additional display sizes
* [ ] Ability to switch solar heating on/off (requires interactive mode)
* [ ] Ability to switch pool pump on/off (requires interactive mode)
* [ ] Web dashboard (served from ESP32 during configuration mode)

## Guides

* [Users Guide](docs/users-guide.md) — Setup, configuration, display layout
* [Hardware Guide](docs/hardware-guide.md) — Board variants, pinout, solar powering
* [Software Guide](docs/software-guide.md) — Development, build, MQTT topics

## Architecture

The firmware follows the same patterns as the Pool Controller for consistency:

- **PoolMonitorContext** — main singleton that owns all subsystems
- **SystemMonitor** — watchdog, memory monitoring, boot-loop detection
- **NetworkManager** — WiFi and MQTT connection management
- **DisplayManager** — E-Ink display control
- **OtaUpdater** — OTA firmware update checker and installer
- **TimeClientHelper** — NTP time sync and timezone support

Key design principles: RAII resource management, static method access for
subsystems, dependency injection via `Preferences` handles, stack-allocated
buffers for memory safety, and graceful degradation on errors.

## Quality Checks

Run the local quality gates before pushing changes:

* Install Python-based linters once: `python3 -m pip install --user cpplint yamllint`
* `markdownlint-cli2` and `jscpd` are executed on demand via `npx`
* `make lint` runs Super-Linter via Docker (see [CONTRIBUTING.md](CONTRIBUTING.md))
* Stage your changes first, then run:

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

## Discussions

See [GitHub Discussions](https://github.com/smart-swimmingpool/smart-swimmingpool.github.io/discussions).

## License

[MIT](LICENSE)

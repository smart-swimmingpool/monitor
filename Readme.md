# Pool Monitor | 🏊 Smart Swimmingpool

[![Smart Swimmingpool](https://img.shields.io/badge/%F0%9F%8F%8A%20-Smart%20Swimmingpool-blue.svg)](https://github.com/smart-swimmingpool)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[![works with Home Assistant](https://img.shields.io/badge/works%20with-Home%20Assistant-41BDF5.svg?logo=home-assistant&logoColor=white "works with Home Assistant")](https://www.home-assistant.io/)

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/J3J33A8DT)

The _Pool Monitor_ is a small additional device to show current pool data using e-ink displays:

* Temperature of the pool water
* Temperature of solar storage
* Pool pump status (on/off)
* Solar heating status (on/off)

## Features

* [x] configurable MQTT server
* [x] automatically connect to MQTT messages of pool controller
* [x] open hotspot (captivate / hotspot) to configure WiFi and MQTT server
* [x] [Home Assistant](https://www.home-assistant.io/) MQTT Discovery compatible (reads pool-controller state topics)
* [x] Timesync via NTP (europe.pool.ntp.org)

## Planned Features

* [ ] solar powering using solar panel
* [ ] nice case to place outdoor at pool
* [ ] support of different displays
* [ ] ability to switch on/off solar heating
* [ ] ability to switch on/off pool pump
* [ ] ability to change controller mode (auto, manual , ...)
* [ ] ability to configure pool controller

## Guides

* [Users Guide](docs/users-guide.md)
* [Hardware Guide](docs/hardware-guide.md)
* [Software Guide](docs/software-guide.md)
* [Home Assistant Migration Notes](docs/home-assistant-migration.md)

## Quality Checks

Run the local quality gates before pushing changes so lint findings are fixed before CI:

* Install Python-based linters once with `python3 -m pip install --user cpplint yamllint`
* `markdownlint-cli2` and `jscpd` are executed on demand via `npx`
* Run the following snippet in Bash

```bash
platformio check --environment LILYGO_T5_V231 --skip-packages
platformio run --environment LILYGO_T5_V231
mapfile -t cpp_files < <(git diff --name-only -- '*.cpp' '*.hpp' '*.h')
((${#cpp_files[@]})) && cpplint "${cpp_files[@]}"
mapfile -t md_files < <(git diff --name-only -- '*.md')
((${#md_files[@]})) && npx --yes markdownlint-cli2 "${md_files[@]}"
mapfile -t yaml_files < <(git diff --name-only -- '*.yml' '*.yaml')
((${#yaml_files[@]})) && yamllint "${yaml_files[@]}"
npx --yes jscpd --config .jscpd.json src .github/workflows
python -m json.tool .jscpd.json > /dev/null
```

## Discussions

see: <https://github.com/smart-swimmingpool/smart-swimmingpool.github.io/discussions>

## License

[LICENSE](LICENSE)

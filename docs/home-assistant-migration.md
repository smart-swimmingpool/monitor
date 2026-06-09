---
title: Home Assistant Migration Notes for Pool Monitor
summary: Developer notes for migrating the Pool Monitor from Homie-based MQTT topics to the Home Assistant setup used by pool-controller
date: "2026-06-09"
lastmod: "2026-06-09"
draft: false
toc: true
type: docs
tags: ["docs", "monitor", "home-assistant", "mqtt", "migration"]
---

# Home Assistant Migration Notes

## Context

The `pool-controller` project has already been moved to Home Assistant MQTT Discovery.
The `monitor` project has now been migrated from Homie-style topic discovery to direct
Home Assistant state topic subscriptions.

This document captures the migration details for reference.
The migration has been completed on branch `feat/home-assistant-migration`.

## Previous monitor behavior (pre-migration)

The implementation used to assume Homie-style discovery:

- subscribed to `homie/+/$name` while searching for the controller
- cached the discovered `device_id`
- subscribed to `homie/<device_id>/#`
- parsed Homie-style subtopics such as:
  - `/pool-temp/temperature`
  - `/solar-temp/temperature`
  - `/pool-pump/switch`
  - `/solar-pump/switch`
  - `/operation-mode/mode`

The migration replaces all of the above with direct Home Assistant state topic subscriptions.

## Target state with Home Assistant

The pool controller now publishes MQTT Discovery entities for Home Assistant.
For a read-only monitor like this one, the safest migration path is to subscribe directly to the
retained Home Assistant state topics instead of trying to rediscover the device via Homie.

Recommended state topics from `pool-controller`:

| Data item | Current Homie topic | Home Assistant state topic |
| --- | --- | --- |
| Pool temperature | `homie/<device_id>/pool-temp/temperature` | `homeassistant/sensor/pool-controller/pool-temp/state` |
| Solar temperature | `homie/<device_id>/solar-temp/temperature` | `homeassistant/sensor/pool-controller/solar-temp/state` |
| Pool pump status | `homie/<device_id>/pool-pump/switch` | `homeassistant/switch/pool-controller/pool-pump/state` |
| Solar pump status | `homie/<device_id>/solar-pump/switch` | `homeassistant/switch/pool-controller/solar-pump/state` |
| Operation mode | `homie/<device_id>/operation-mode/mode` | `homeassistant/select/pool-controller/mode/state` |

Notes:

- The monitor only needs the `state` topics; it does not need any `set` topics.
- If the published payload format changes from the old Homie representation, the callback parsing must be updated too.
- The current logic that searches for a `device_id` should be removed or replaced with a fixed HA topic list.

## Implementation completed

1. ✅ Deep-sleep and display pipeline kept unchanged.
2. ✅ Homie discovery logic replaced with Home Assistant topic subscriptions.
3. ✅ Fixed state topics subscribed; retained messages provide data immediately after wake-up.
4. ⚠️ Clear stale retained Homie messages on the broker after deploying this firmware.
5. ✅ Monitor remains read-only.

## Library versions used in this checkout

The `platformio.ini` pins and the actually resolved versions in `.pio/libdeps/LILYGO_T5_V231/` are not always identical.
Below is the state of the current checkout.

| Library | Pin in `platformio.ini` | Resolved version in `.pio/libdeps` |
| --- | --- | --- |
| espressif32 platform | `^6.5.0` | build platform only, not stored in `library.properties` |
| Adafruit BusIO | `^1.16.1` | `1.17.4` |
| Adafruit GFX Library | `^1.11.11` | `1.12.6` |
| GxEPD | `^3.1.1` | `3.1.3` |
| ESP-WiFiSettings | `^3.9.2` | `3.10.1` |
| U8g2 | `^2.35.30` | `2.36.18` |
| U8g2_for_Adafruit_GFX | `1.8.0` | `1.8.0` |
| PubSubClient | `2.8` | `2.8` |
| NTPClient | `3.2.1` | `3.2.1` |
| mDNSResolver | `0.3` | `0.3` |
| Timezone | `1.2.4` | `1.2.6` |
| Time | not pinned directly | `1.6.1` |

## Practical takeaway

The monitor is now decoupled from Homie. It subscribes directly to the pool-controller's
Home Assistant state topics and no longer performs Homie discovery.
The pool-controller side remains on Home Assistant Discovery, and the monitor now consumes
the HA state topics directly.

---
title: Users Guide of Pool Monitor
summary: Set up and configure the Pool Monitor — WiFi, MQTT broker, captive portal, display layout, and OTA updates
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

## Setup

When the device is powered on for the first time (or after a factory reset),
it starts a **WiFi hotspot** named `pool-monitor`.

1. Connect your smartphone or laptop to the `pool-monitor` WiFi network (open,
   no password).
2. A **captive portal** should open automatically. If not, open a browser and
   go to **`http://192.168.4.1`**.
3. Enter the following:
   - **WiFi SSID** and **password**
   - **MQTT broker hostname** or IP address
   - **MQTT port** (default: `1883`)
4. Click **Save** — the device reboots and connects to your network.

After a few seconds, the E-Ink display updates with the current pool data.

### MQTT Broker

The Pool Monitor reads data published by the
[Pool Controller](https://github.com/smart-swimmingpool/pool-controller)
via Home Assistant MQTT state topics. Your MQTT broker must be reachable from
the same WiFi network as the monitor.

---

## Display Layout

The E-Ink display shows the following information:

```text
┌──────────────────────────────────┐
│ 🌊  Pool Temperature    25.3 °C │
│ ☀️  Solar Temperature   55.1 °C │
│                                  │
│ ⏱️  Pump Pool           ON      │
│ ⏱️  Solar Pump          ON      │
│ ⏱️  Mode                auto    │
│                                  │
│         14:30                    │
│    www.smart-swimmingpool.com    │
└──────────────────────────────────┘
```

- **Temperatures** are updated whenever new MQTT data arrives.
- **Pump status** shows `ON` / `OFF` based on the pool controller state.
- **Mode** shows the current operation mode (`auto`, `manual`, etc.).
- **Time** is synchronised via NTP (hourly) with automatic CET/CEST.
- **Branding** at the bottom shows the project website.

The display only updates when data changes — it stays unchanged during deep
sleep to save energy.

---

## Network Configuration (mDNS)

The device is reachable on your local network as `pool-monitor.local` via mDNS.
This works on most networks without any configuration.

The DHCP lease is preserved across deep sleep cycles — the device does not
release its IP address when going to sleep. It remains visible in the router's
client table (though unreachable while sleeping).

---

## MQTT Errors and Configuration Portal

If the MQTT broker cannot be reached (wrong hostname, temporary outage, etc.),
the device automatically starts a **configuration portal**:

1. The display shows:
   - WiFi SSID (`pool-monitor`)
   - Access point IP (`192.168.4.1`)
   - A QR code for quick access
2. Connect to the `pool-monitor` WiFi network (open, no password).
3. Open the configuration page — either scan the QR code or go to
   `http://192.168.4.1`.
4. Correct the MQTT settings (hostname, port) or WiFi credentials.
5. Click **Save** — the device reboots and retries the connection.

> **DNS tip**: If the MQTT hostname (e.g. `smarthome-pi`) cannot be resolved,
> enter the IP address instead. The hostname is still attempted without DNS
> (PubSubClient resolves internally), but a direct IP is more reliable.

The portal stays active for **5 minutes**. If no configuration is saved within
this time, the device enters deep sleep to conserve power. It retries the
connection on the next wake cycle (every 180 seconds).

---

## OTA Firmware Updates

The Pool Monitor supports **over-the-air (OTA) firmware updates** via GitHub
Releases:

- On each wake cycle, the device checks GitHub for a newer firmware version.
- If an update is available, it downloads the `.bin` release asset and flashes
  it automatically.
- The display briefly shows "Update" during the process.
- After the update, the device reboots and resumes normal operation.

No manual intervention is required — the update happens fully automatically
during a regular wake cycle. To trigger an immediate check, simply power-cycle
the device.

---

## Time Synchronisation

The device synchronises its clock via NTP (Network Time Protocol) using
`europe.pool.ntp.org`:

- **NTP sync** occurs approximately every **60 minutes**.
- Between syncs, the time is reconstructed from the last known time plus
  elapsed uptime.
- The display always shows **local time** with automatic daylight saving time
  switching (CET/CEST).

If the time shows `--:--` on the display, NTP sync has not succeeded yet.
This typically resolves on the next wake cycle (within 3 minutes).

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| Display stays white | No firmware or corrupted flash | Flash firmware via PlatformIO (see [Software Guide](software-guide.md)) |
| "MQTT Error" on display, portal opens | MQTT broker unreachable | Check broker hostname/IP; verify MQTT server is running |
| "WiFi connection failed" | Wrong credentials or weak signal | Reconfigure via captive portal; check WiFi range |
| Display shows old data (never updates) | MQTT issue or pool controller offline | Verify pool controller publishes to HA state topics |
| QR portal appears every 3 minutes | MQTT settings not saving | Re-enter MQTT hostname/IP; try IP address instead of hostname |
| Brownout / reboot loop | Insufficient power supply | Use 5V/≥1A supply; try different USB cable |
| Display updates but shows `--:--` | NTP time sync failed | Check internet connectivity; NTP is attempted every hour |

---

## References

- [Hardware Guide](hardware-guide.md) — Board assembly, pinout, solar powering
- [Software Guide](software-guide.md) — Development setup, build, MQTT topics
- [Pool Controller](https://github.com/smart-swimmingpool/pool-controller)
- [Home Assistant](https://www.home-assistant.io/)
- [Smart Swimming Pool Website](https://www.smart-swimmingpool.com)

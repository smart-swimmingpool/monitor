---
title: Users Guide of Pool Controller
summary: Control your Smart Swimming Pool smart
date: "2022-06-11"
lastmod: "2022-06-11"
draft: false
toc: true
type: docs
featured: true
tags: ["docs", "monitor", "tutorial"]
menu:
  docs:
    parent: Pool Monitor
    name: Users Guide
    weight: 40
---

## Setup

Connecting the device to power it createsinitial a hotspot called "pool-monitor".
Please connect this hotsppot by smartphone. Thenthe device will show captivate website to add
WiFi credentials and connection parameters of MQTT broker.

Following data is required to connect the device:

* SSID of the WiFi network
* Password of WiFi network
* MQTT broker hostname or IP address
* MQTT broker port number (default: 1883)

After pressing "Save" the device will reboot automatically and tries to connect to the MQTT broker.

After some seconds the device will show data if connection was successful.

## Network Configuration (mDNS)

The device is discoverable on the local network as `pool-monitor.local` via mDNS. This allows finding it without knowing its IP address.

The DHCP lease is preserved across deep sleep cycles — the device does not release its IP address when going to sleep. It remains visible in the router's client table (though unreachable while sleeping).

## MQTT Errors and Configuration Portal

If the MQTT broker cannot be reached (e.g. wrong hostname, temporary outage), the device automatically starts a WiFi hotspot with a configuration portal:

1. The display shows the WiFi SSID (`pool-monitor`), the access point IP (`192.168.4.1`), and a QR code.
2. Connect to the `pool-monitor` WiFi network (open network, no password required).
3. Open the configuration page — either by scanning the QR code or navigating to `http://192.168.4.1`.
4. Correct the MQTT settings (hostname, port).
5. Click "Save" — the device reboots and retries the MQTT connection.

**Important for DNS issues:** If the MQTT hostname (e.g. `smarthome-pi`) cannot be resolved, use the IP address of the MQTT broker instead. The hostname is still attempted even without DNS resolution (PubSubClient resolves internally), but an IP address is more reliable.

The portal stays active for **5 minutes**. If no configuration is saved within this time, the device enters deep sleep to conserve battery. On the next wake cycle (every 180 seconds), it retries the MQTT connection.

## Display During Configuration Portal

When an MQTT error occurs, the display shows:

- WiFi network name (`pool-monitor`)
- Access point IP address (`192.168.4.1`)
- QR code for instant connection via smartphone camera

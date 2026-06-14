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

## Einrichtung

Wenn das Gerät mit Strom verbunden wird, erstellt es initial einen Hotspot namens "pool-monitor".
Bitte verbinde dich per Smartphone mit diesem Hotspot. Das Gerät zeigt dann eine Captive-Website an, um
WLAN-Zugangsdaten und Verbindungsparameter des MQTT-Brokers einzugeben.

Folgende Daten werden für die Verbindung benötigt:

* SSID des WLAN-Netzwerks
* Passwort des WLAN-Netzwerks
* MQTT-Broker-Hostname oder IP-Adresse
* MQTT-Broker-Portnummer (Standard: 1883)

Nach dem Klick auf "Save" startet das Gerät automatisch neu und versucht, sich mit dem MQTT-Broker zu verbinden.

Nach einigen Sekunden zeigt das Gerät die Daten an, wenn die Verbindung erfolgreich war.

## Netzwerkkonfiguration (mDNS)

Das Gerät ist im Netzwerk unter `pool-monitor.local` per mDNS erreichbar. So kann es auch ohne bekannte IP-Adresse gefunden werden.

Der DHCP-Lease bleibt über Deep-Sleep-Zyklen hinweg erhalten — das Gerät gibt seine IP-Adresse nicht frei, wenn es in den Schlaf geht. Es bleibt in der Router-Tabelle sichtbar (wenn auch nicht erreichbar, solange es schläft).

## MQTT-Fehler und Konfigurationsportal

Kann der MQTT-Broker nicht erreicht werden (z.B. falscher Hostname, temporärer Ausfall), startet das Gerät automatisch einen WLAN-Hotspot mit Konfigurationsportal:

1. Auf dem Display erscheinen die WLAN-SSID (`pool-monitor`), die IP-Adresse des Zugangspunkts (`192.168.4.1`) sowie ein QR-Code.
2. Verbinde dich mit dem WLAN `pool-monitor` (offenes Netz, kein Passwort).
3. Öffne die Konfigurationsseite — entweder über den QR-Code oder unter `http://192.168.4.1`.
4. Korrigiere die MQTT-Einstellungen (Hostname, Port).
5. Klicke "Save" — das Gerät startet neu und verbindet sich erneut.

**Wichtig bei DNS-Problemen:** Wenn der MQTT-Hostname (z.B. `smarthome-pi`) nicht aufgelöst werden kann, verwende stattdessen die IP-Adresse des MQTT-Brokers. Der Hostname wird auch ohne DNS-Auflösung versucht (PubSubClient löst selbst auf), aber eine IP-Adresse ist zuverlässiger.

Das Portal bleibt **5 Minuten** aktiv. Wenn in dieser Zeit keine Konfiguration gespeichert wird, wechselt das Gerät in den Tiefschlaf, um die Batterie zu schonen. Beim nächsten Aufwachen (alle 180 Sekunden) wird die MQTT-Verbindung erneut versucht.

## Display während des Konfigurationsportals

Wenn der MQTT-Fehler auftritt, zeigt das Display:

- WLAN-Name (`pool-monitor`)
- IP-Adresse des Zugangspunkts (`192.168.4.1`)
- QR-Code zum direkten Verbinden per Smartphone-Kamera

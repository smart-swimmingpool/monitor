---
linktitle: Pool Monitor
summary: Überwache die Temperatur deines Swimmingpools

# page metadata.
title: Pool Monitor
date: "2020-05-28"
lastmod: "2026-06-27"
draft: false
toc: true
type: docs
featured: true

tags: ["docs", "esp32", "tutorial"]

menu:
  docs:
    parent: Pool Monitor
    name: Überblick
    weight: 10
---

<span style="text-shadow: none;">
<a class="github-button" href="https://github.com/smart-swimmingpool/monitor/subscription" data-size="large" data-show-count="true" aria-label="Watch smart-swimmingpool/monitor on GitHub">Watch</a>
<a class="github-button" href="https://github.com/smart-swimmingpool/monitor" data-icon="octicon-star" data-size="large" data-show-count="true" aria-label="Star this on GitHub">Star</a><script async defer src="https://buttons.github.io/buttons.js"></script>
</span>

Der [Pool Monitor](https://github.com/smart-swimmingpool/monitor) ist ein dediziertes E-Ink-Anzeigegerät, das aktuelle Pool-Daten anzeigt — Temperatur, Pumpen-Status und Solar-Status — durch Abonnement der Home-Assistant-MQTT-Topics des [Pool Controllers](https://github.com/smart-swimmingpool/pool-controller).

{{< figure library="true" src="pool-monitor-prototype.jpg" title="Prototype of Pool Monitor" lightbox="true" >}}

[![works with Home Assistant](https://img.shields.io/badge/works%20with-Home%20Assistant-41BDF5.svg?logo=home-assistant&logoColor=white "works with Home Assistant")](https://www.home-assistant.io/)

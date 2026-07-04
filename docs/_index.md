---
linktitle: Pool Monitor
summary: Monitor the temperature of your swimming pool

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
    name: Overview
    weight: 10
---

<span style="text-shadow: none;">
<a class="github-button" href="https://github.com/smart-swimmingpool/monitor/subscription" data-size="large" data-show-count="true" aria-label="Watch smart-swimmingpool/monitor on GitHub">Watch</a>
<a class="github-button" href="https://github.com/smart-swimmingpool/monitor" data-icon="octicon-star" data-size="large" data-show-count="true" aria-label="Star this on GitHub">Star</a><script async defer src="https://buttons.github.io/buttons.js"></script>
</span>

The [Pool Monitor](https://github.com/smart-swimmingpool/monitor) is a dedicated E-Ink display device that shows current swimming pool data — temperature, pump status, and solar status — by subscribing to Home Assistant MQTT topics from the [Pool Controller](https://github.com/smart-swimmingpool/pool-controller).

{{< figure library="true" src="pool-monitor-prototype.jpg" title="Prototype of Pool Monitor" lightbox="true" >}}

[![works with Home Assistant](https://img.shields.io/badge/works%20with-Home%20Assistant-41BDF5.svg?logo=home-assistant&logoColor=white "works with Home Assistant")](https://www.home-assistant.io/)

# MeshCoreTel Firmware

This is a fork of MeshCore-EastMesh firmware with MeshCoreTel broker support instead of EastMesh.

MeshCoreTel:

- <https://meshcoretel.ru/>

Original firmware (credits to Jared Dohrman, Scott Powell / Ripple Radios and the MeshCore contributors):

- <https://github.com/xJARiD/MeshCore-EastMesh>

MeshCoreTel firmware builds for MeshCore, with MQTT repeaters, WiFi companions, and simple release downloads for supported boards.

This repository keeps the upstream MeshCore firmware intact and adds an MeshCoreTel layer on top for packaging, release automation, WiFi-enabled companion builds, and MQTT-enabled repeater builds.

## Key MeshCoreTel Features

### MQTT Repeater Additions

- hardcoded broker support for:
  - `meshcoretel`
  - `letsmesh-eu`
  - `letsmesh-us`
- WSS transport at `/mqtt`
- JWT auth using the device identity
- CLI controls for:
  - WiFi credentials
  - WiFi powersaving
  - board battery reporting on supported targets
  - MQTT endpoint enablement
  - MQTT packet and raw publishing
  - owner public key and email
  - local web panel enablement

### Local Web Panel

On supported `*_repeater_mqtt` ESP32 targets, the repeater can expose a local HTTPS config panel over WiFi.

Features include:

- password-gated access using the existing repeater admin password
- allowlisted CLI execution
- grouped quick actions
- light and dark themes
- optional disable via `set web off`

Recommended use is initial setup and occasional troubleshooting. On MQTT repeater deployments, disable the panel again when finished if you want maximum MQTT heap headroom.

### Companion WiFi Additions

`*_companion_radio_wifi` targets now support persisted WiFi rescue commands:

- open a serial monitor at `115200` baud
- reboot the device
- long-press the user button within the first 8 seconds after boot to enter `CLI Rescue`
- wait for `========= CLI Rescue =========`
- send the commands from the serial monitor

These rescue commands are only available after entering `CLI Rescue`:

- `get wifi.status`
- `get wifi.ssid`
- `get wifi.powersaving`
- `set wifi.ssid <ssid>`
- `set wifi.pwd <password>`
- `set wifi.powersaving none|min|max`

## Status

Tested with my Heltec V4 OLED.

## Documentation

Current docs pages:

- [Home](./docs/index.md)
- [Custom CLI Commands](./docs/custom-cli.md)

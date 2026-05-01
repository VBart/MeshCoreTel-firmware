# MeshCoreTel-firmware Docs

MeshCoreTel-firmware keeps the upstream MeshCore firmware intact and adds an additional layer on top for:

- `repeater_mqtt` builds with native WiFi, MQTT, and the optional local web panel
- `companion_radio_wifi` builds for Wi-Fi-connected companion devices
- MeshCoreTel-firmware release automation and versioning on top of upstream MeshCore releases

If you are just trying to pick a board or download firmware, start with:

- [Compare Boards](./boards.md)
- [Download and Flash Releases](./releases.md)

## End User Guides

- [Compare Boards](./boards.md)
- [Download and Flash Releases](./releases.md)
- [Migration From xJARiD/MeshCore](./migration.md)
- [Use the Repeater Web Panel](./web-panel.md)
- [Use the Repeater Web API](./api.md)
- [Custom CLI Commands](./custom-cli.md)

## Developer Notes

- [Build Locally With uv](./local-builds.md)

## Current Scope

This docs site only covers the MeshCoreTel-firmware specific pieces in this repository.

For general MeshCore behavior, radio operation, and upstream firmware concepts, refer to the upstream project:

- [meshcore-dev/MeshCore](https://github.com/meshcore-dev/MeshCore)

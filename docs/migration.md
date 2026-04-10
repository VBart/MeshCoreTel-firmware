# Migration From `xJARiD/MeshCore`

This page covers migrating from the previous firmware published at:

- `https://github.com/xJARiD/MeshCore`

to the EastMesh firmware published at:

- `https://github.com/xJARiD/MeshCore-EastMesh`

These steps are intended for existing repeater users moving to the new EastMesh `*_repeater_mqtt` firmware.

Start by downloading the correct EastMesh release for your board from:

- [Download and Flash Releases](./releases.md)

## Before You Start

- you can flash the new firmware without erasing first
- after flashing, you will need to reapply your Wi-Fi and some MQTT settings
- you can do this either from the serial CLI or from the companion app

## Required Setup After Flashing

Once the new firmware is flashed, set:

- `set wifi.ssid <your-ssid>`
- `set wifi.pwd <your-password>`
- `set mqtt.iata <code>`
- `set mqtt.status on`

## Optional Settings

If you were previously using owner metadata, also set:

- `set mqtt.owner <64-hex-char-public-key>`
- `set mqtt.email <email>`

If you had previously disabled TX publishing, also set:

- `set mqtt.tx off`

## Final Step

Once the settings are applied:

- `reboot`

After reboot, the repeater should be operational.

## Verify Operation

Confirm the repeater is up and connected with:

- `get wifi.status`
- `get mqtt.status`
- `get mqtt.statuscfg`

## Validate In The Web Console

If the web panel is enabled on your board, you can also verify through the local HTTPS admin page.

Enable and check it with:

- `set web on`
- `get web.status`

Then open:

- `https://<IP_ADDRESS>/`

Notes:

- ignore the browser warning for the self-signed certificate
- log in with the admin password
- this is the same admin password used for admin access via the companion app

## Legacy MQTT Commands

Some MQTT CLI commands from the previous firmware do not apply in EastMesh and should not be migrated.

The previous firmware stored its MQTT bridge settings in a different place and shape than EastMesh does now. In the old firmware, `MQTTBridge` used legacy node preferences for items such as broker host, port, analyzers, TX enable, owner key, email, and IATA. In EastMesh, the current Wi-Fi and MQTT uplink settings are stored separately in `/mqtt_prefs`.

These previous MQTT CLI commands no longer apply in EastMesh:

- `get mqtt.server`
- `get mqtt.port`
- `get mqtt.password`
- `get mqtt.analyzer.us`
- `get mqtt.analyzer.eu`
- `get mqtt.config.valid`

If you flash without erase first, older legacy values may still remain in flash in the background. The important point is that EastMesh does not use those old MQTT bridge values for its current uplink configuration, so users should not expect them to carry forward or have any effect.

## Recommended Migration Flow

1. Flash the new EastMesh firmware without erasing.
2. Open the serial CLI or companion app.
3. Set `wifi.ssid`, `wifi.pwd`, `mqtt.iata`, and `mqtt.status on`.
4. Optionally set `mqtt.owner` and `mqtt.email`.
5. If you previously had TX publishing disabled, set `mqtt.tx off`.
6. Reboot the repeater.
7. Verify with `get wifi.status`, `get mqtt.status`, and `get mqtt.statuscfg`.
8. Optionally confirm local web access with `set web on`, `get web.status`, and `https://<IP_ADDRESS>/`.

## Related Docs

- [Repeater Web Panel](./web-panel.md)
- [Custom CLI Commands](./custom-cli.md)
- [Download and Flash Releases](./releases.md)

## Last Resort

If the repeater still does not come up cleanly after reflashing, reapplying settings, and rebooting, the last resort is to perform a full erase-flash and then configure it again from scratch.

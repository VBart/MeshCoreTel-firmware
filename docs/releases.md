# Download And Flash Releases

EastMesh release assets are published on:

- <https://github.com/xJARiD/MeshCore-EastMesh/releases>

There are currently two release tracks in this repo:

- `companion-wifi`
- `repeater-mqtt`

## Pick The Right Asset

Download the asset that matches your board and firmware type.

Examples:

- `heltec_v4_companion_radio_wifi-v1.14.1-abcdef.bin`
- `heltec_v4_repeater_mqtt-v1.14.1-eastmesh-v1.0.1-abcdef.bin`
- `heltec_v4_repeater_mqtt-v1.14.1-eastmesh-v1.0.1-abcdef-merged.bin`

The important part is the board/env prefix:

- `*_companion_radio_wifi`
- `*_repeater_mqtt`

## Which File To Flash

Use the standard `.bin` file when you are updating an existing device with the same target and partition layout.

Use the `-merged.bin` file when you want a clean install after erasing flash. This is the full ESP32 image and is intended to be flashed from address `0x0`.

Practical rule:

- `.bin` = incremental update
- `-merged.bin` = erase and clean flash

## Flashing Flow

1. Open the release page and download the file for your board.
2. Confirm the board name in the filename matches your hardware.
3. Choose one of the following:
   - update existing firmware with the normal `.bin`
   - erase the device first, then flash the `-merged.bin`
4. Reboot the device and complete any post-flash setup such as WiFi, MQTT, or radio settings.

## Recommended Flasher

The recommended flasher is:

- <https://flasher.eastmesh.au/>

It includes native support for:

- `companion_radio_wifi` firmware
- `repeater_mqtt` firmware
- custom firmware files

Recommended usage:

- use the normal `.bin` there when you are updating an existing device
- use the `-merged.bin` there after an erase when you want a clean flash

## Beginner Setup

If this is your first time flashing EastMesh firmware, the easiest path is:

1. Open <https://flasher.eastmesh.au/>.
2. Select the firmware type you want:
   - `Companion WiFi`
   - `Repeater MQTT`
   - or `Custom`
3. Flash the correct firmware for your board.
4. Use the built-in setup tools in the flasher site to finish first-time configuration.

The flasher site includes two especially useful actions after flashing:

- `Repeater Setup`
- `Console`

### Repeater Setup

`Repeater Setup` is the guided first-time repeater flow.

It is the traditional way to configure a repeater after flashing, including:

- device name
- latitude and longitude
- admin and guest passwords
- radio settings, including preset selection
- advert interval
- flood advert interval
- flood max
- some advanced repeater settings

As of `v1.2.1`, the local repeater web panel also includes the same common repeater settings, so many users can continue day-to-day configuration there after initial setup.

### Console

`Console` is the raw CLI interface.

It is especially useful, and often required, for the initial Wi-Fi setup on both firmware tracks:

- `set wifi.ssid <your-ssid>`
- `set wifi.pwd <your-password>`

This applies to:

- `companion_radio_wifi`
- `repeater_mqtt`

## Repeater MQTT Notes

`repeater_mqtt` builds include the EastMesh MQTT additions. Depending on the board, they may also include the local web panel.

Typical first steps after flashing:

- set `wifi.ssid`
- set `wifi.pwd`
- set `mqtt.iata`
- confirm `get mqtt.status`
- optionally set `mqtt.owner` and `mqtt.email`
- optionally enable `letsmesh-eu` or `letsmesh-us`

## Companion WiFi Notes

`companion_radio_wifi` builds are for companion devices that expose the app interface over WiFi instead of BLE or USB.

Typical first steps after flashing:

- set WiFi credentials
- confirm `get wifi.status`
- connect your client app to the companion over WiFi

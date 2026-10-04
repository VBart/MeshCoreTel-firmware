# WireGuard ESP32 vendoring note

This directory is based on `ciniml/WireGuard-ESP32-Arduino` version 0.1.5 and
retains its BSD 3-Clause license.

MeshCoreTel carries two small local extensions used by the optional repeater
WireGuard target:

- an overload that accepts an IPv4 netmask and gateway, so the WireGuard
  interface can own only the configured tunnel subnet while Wi-Fi remains the
  default route;
- `peer_is_up()` status reporting with session/RX/TX ages for CLI diagnostics.

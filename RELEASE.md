# Releasing Firmware

GitHub Actions is set up to automatically build and draft firmware releases.

## Required GitHub Variable

Set the repository Actions variable `OFFICIAL_MESHCORE_VERSION` to the current upstream MeshCore release version.

Example:

- `OFFICIAL_MESHCORE_VERSION=v1.2.3`

This value becomes the base `FIRMWARE_VERSION` used by the release workflows.

## MeshCoreTel-firmware Release Tags

Push one or more of the following tag formats to trigger the matching firmware release workflow:

- `companion-wifi-v1.2.3`
- `repeater-mqtt-meshcoretel-v1.0.1`

Use the upstream MeshCore version in `companion-wifi-v1.2.3`.
Use the MeshCoreTel-firmware release version in `repeater-mqtt-meshcoretel-v1.0.1`.

Each tag triggers a separate workflow:

- `companion-wifi-v*` builds companion WiFi firmware
- `repeater-mqtt-meshcoretel-*` builds repeater MQTT firmware

You can push one, or more tags on the same commit, and they will all build separately.

## Resulting Firmware Version

During the GitHub Actions build:

- `companion-wifi` uses the version in the tag as `FIRMWARE_VERSION`
- `repeater-mqtt` uses `OFFICIAL_MESHCORE_VERSION` as `FIRMWARE_VERSION`
- `repeater-mqtt` uses the MeshCoreTel-firmware version from the tag as `MESHCORETEL_VERSION`

The resulting version string depends on the workflow:

- `companion-wifi`: `v1.2.3-<commit>`
- `repeater-mqtt`: `v1.2.3-vbart-meshcoretel-v1.0.1-<commit>`

Example:

- tag: `repeater-mqtt-meshcoretel-v1.0.1`
- repo variable: `OFFICIAL_MESHCORE_VERSION=v1.2.3`
- resulting build version: `v1.2.3-meshcoretel-v1.0.1-abcdef`

## Typical Release Flow

1. Update the `OFFICIAL_MESHCORE_VERSION` GitHub Actions variable if upstream has changed.
2. Update `release-notes.yml` on `develop` for any release entries you want included in the tagged commit.
3. Open and merge the release PR from `develop` to `main`.
4. Create the release tags you want on the target commit on `main`.
5. Push the tags.
6. Wait for the workflows to finish building.
7. Review the draft GitHub Releases.
8. Publish the release.

If `release-notes.yml` should reflect the tagged firmware release in-repo, make that change before the PR to `main` so the tagged commit already contains the matching release notes.

Example:

```bash
git tag companion-wifi-v1.2.3
git tag repeater-mqtt-meshcoretel-v1.0.1
git push origin companion-wifi-v1.2.3 repeater-mqtt-meshcoretel-v1.0.1
```

## Supported Tags

- `companion-wifi-v1.2.3`
- `repeater-mqtt-meshcoretel-v1.0.1`

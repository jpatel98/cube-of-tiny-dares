# Waveshare Tiny Dares Firmware

This folder vendors the existing AgentGotchi firmware base for the physical
Cube of Tiny Dares hardware.

Source copied from:

```text
/Users/jigar/Development/AgentGotchi/firmware
/Users/jigar/Development/AgentGotchi/output/imagegen
```

Target hardware:

- Waveshare ESP32-S3-Touch-LCD-1.54
- 240x240 ST7789 LCD
- CST816 capacitive touch
- QMI8658 IMU
- ESP32-S3R8 with 8 MB PSRAM and 16 MB flash
- USB serial device currently appears as `/dev/cu.usbmodem101`

## Why This Exists

The minimal sketch in `hardware/esp32_tiny_dares/` documents the simple
`POST /api/dare` contract, but it does not know about the actual Waveshare
display, touch controller, IMU, or AgentGotchi sprite assets.

This folder is the real hardware path for the hackathon cube. It keeps the
AgentGotchi sprites and display stack, while the active app behavior is Tiny
Dares.

## Assets

Generated LVGL assets live in:

```text
firmware/src/assets/agentgotchi_assets.cpp
firmware/src/assets/agentgotchi_assets.h
```

Original/reference PNGs live in:

```text
assets/
```

Do not delete the AgentGotchi sprites. They are part of the physical cube's
personality. The current flashed UI uses one clean pet sprite rather than
switching sprites per dare.

## Current State

The firmware has been adapted to Cube of Tiny Dares behavior:

- On first boot, it starts a Wi-Fi provisioning portal named
  `TinyDares-setup` if no saved network is available.
- Provisioning accepts the Tiny Dares `/api/dare` endpoint, default context,
  mode, and intensity.
- The default endpoint is the live Build Small Hackathon Space:
  `https://build-small-hackathon-cube-of-tiny-dares.hf.space/api/dare`.
- Saved old local endpoints are migrated back to the live default on boot.
- A screen tap or KEY press posts JSON to `/api/dare`.
- The firmware keeps the last few successful dare texts in RAM and sends them
  as `recent` so repeated taps avoid immediate repeats.
- The display renders `cube.display` and `cube.color` with one AgentGotchi pet
  sprite.
- `cube.timer_seconds` is still accepted as part of the API contract, but the
  physical screen does not show a countdown.
- Holding KEY clears saved provisioning and restarts the device.

The firmware request body is:

```json
{
  "context": "I am stuck and need a tiny dare",
  "mode": "builder",
  "intensity": "medium",
  "recent": [
    "Stop researching. Build the dumbest visible version."
  ]
}
```

The firmware reads:

- `cube.display`
- `cube.color`
- `cube.timer_seconds`
- one AgentGotchi pet sprite

## TLS / HTTPS

The firmware makes three HTTPS connections:

| Endpoint | Host | TLS verification |
|---|---|---|
| Dare endpoint (`/api/dare`) | Hugging Face Space / Modal | **Verified** against Amazon Root CA 1 and ISRG Root X1 |
| Geolocation | ipapi.co | Unverified — data is non-sensitive (city/lat/lon/timezone) |
| Weather | api.open-meteo.com | Unverified — data is non-sensitive (temperature/conditions) |

The dare endpoint CA bundle is embedded in `firmware/src/tls_roots.h`.
It includes:

- Amazon Root CA 1 for the default Hugging Face Space endpoint. It expires
  2038-01-17.
- ISRG Root X1 for the Modal endpoint. It expires 2035-06-04.

Update this bundle if Hugging Face or Modal rotate certificate roots.

### Development override

If you are testing against a local endpoint that uses a self-signed
certificate, add the following flag to your PlatformIO build:

```ini
; platformio.ini  [env:tiny_dares]
build_flags = -DTINY_DARES_ALLOW_INSECURE_TLS
```

This disables certificate verification for the dare endpoint only and falls
back to `setInsecure()`.  **Do not ship firmware with this flag set.**

## Build

From this folder:

```bash
cd firmware
pio run
```

The compile check has passed locally with PlatformIO.

## Upload

Upload requires the connected Waveshare board. The detected local serial device
was:

```text
/dev/cu.usbmodem101
```

The last verified upload command was:

```bash
cd firmware
pio run -t upload --upload-port /dev/cu.usbmodem101
```

After flashing, the default endpoint is:

```text
https://build-small-hackathon-cube-of-tiny-dares.hf.space/api/dare
```

To override it, connect to the `TinyDares-setup` Wi-Fi portal and set a local or
alternate hosted `/api/dare` URL.

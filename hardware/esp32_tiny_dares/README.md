# ESP32 Tiny Dares Integration

Keep the hardware dumb.

## Contract

On tap/button press:

1. POST JSON to the app:

```http
POST https://YOUR-SPACE.hf.space/api/dare
Content-Type: application/json

{"context":"I keep researching and can't pick a direction"}
```

2. Parse response:

```json
{
  "cube": {
    "display": "Stop researching for 20 minutes. Build the dumbest visible version.",
    "color": "#FFB703",
    "timer_seconds": 1200
  }
}
```

3. Show:

- `cube.display` on LCD.
- `cube.color` as LED/background if available.
- Optional countdown using `timer_seconds`.

## Why not Gradio API directly?

Gradio event endpoints use submit/poll semantics. That is annoying on an ESP32.

This project exposes `/api/dare` specifically so the cube can make **one simple HTTP request**.

## Arduino dependencies

- WiFi.h
- HTTPClient.h
- ArduinoJson

Install ArduinoJson from the Arduino Library Manager.

## Recommended sketch wiring (generic)

The sketch keeps I/O assumptions minimal so it can compile across common ESP32 boards:

- Set `BUTTON_PIN` to a free GPIO for your physical trigger.
- If your button is wired active-low (most common with INPUT_PULLUP), leave
  `BUTTON_ACTIVE_LOW` as `true`.
- If you wire it active-high, set `BUTTON_ACTIVE_LOW` to `false` and keep
  `pinMode(BUTTON_PIN, INPUT_PULLDOWN)` (or match your wiring).
- Compile with Serial enabled first to validate the endpoint and payload with
  log output before adding a display.

## Runtime checklist

1. Configure:
  - `WIFI_SSID`
  - `WIFI_PASSWORD`
  - `DARE_ENDPOINT`
2. Fill in any display/touch hardware code inside `showDare()` only.
3. Flash and open Serial Monitor at 115200.
4. Press the button once. Confirm logs show:
  - successful Wi‑Fi connect (or clear timeout message),
  - HTTP status,
  - `cube.display`, `cube.color`, and `cube.timer_seconds`.

If those values print correctly, the wire protocol is ready for your physical
cube display integration.

## Scope warning

Do not start with audio recording or streaming. First version should be:

button → POST → text/color on screen

That is the whole magic.

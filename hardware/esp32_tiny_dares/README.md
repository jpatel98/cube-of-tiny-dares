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

## Scope warning

Do not start with audio recording or streaming. First version should be:

button → POST → text/color on screen

That is the whole magic.

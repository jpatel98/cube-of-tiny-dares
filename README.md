---
title: Cube of Tiny Dares
emoji: 🎲
colorFrom: purple
colorTo: yellow
sdk: gradio
app_file: app.py
pinned: false
license: mit
python_version: "3.11"
---

<div align="center">
  <img src="assets/social-card.svg" alt="Cube of Tiny Dares — tap the cube, get one tiny dare" width="100%" />

  <h1>🎲 Cube of Tiny Dares</h1>

  <p><strong>Tap the cube. Get one tiny dare. Move.</strong></p>

  <p>
    <a href="https://github.com/jpatel98/cube-of-tiny-dares/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/jpatel98/cube-of-tiny-dares/actions/workflows/ci.yml/badge.svg" /></a>
    <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/license-MIT-blue.svg" /></a>
    <img alt="Python 3.11+" src="https://img.shields.io/badge/python-3.11%2B-3776AB.svg" />
    <img alt="Built with Gradio" src="https://img.shields.io/badge/built%20with-Gradio-orange.svg" />
    <img alt="ESP32 friendly" src="https://img.shields.io/badge/ESP32-friendly-6A5ACD.svg" />
  </p>
</div>

---

**Cube of Tiny Dares** is a tiny AI-appliance-shaped hackathon project for getting unstuck.

You tell it what loop you are in. You tap the cube. It gives **one tiny dare**:

- “Delete one feature.”
- “Ship the fake version first.”
- “Ask one human to try the ugly version today.”
- “Stop researching for 20 minutes.”

No dashboard. No productivity cosplay. No account system. Just a playful physical nudge toward motion.

## The vibe

Most builder tools ask you to manage more things.

This one asks you to do **one smaller thing**.

```text
context → tap → one tiny dare → move
```

## Demo examples

| Context | Tiny dare |
| --- | --- |
| “I keep researching models and can't pick a direction.” | “Stop researching for 20 minutes. Build the dumbest visible version.” |
| “I want to add login before the demo.” | “Delete one feature. If the demo still works, it was not core.” |
| “The deploy failed and I am randomly changing stuff.” | “Reproduce the failure once. Screenshot/log it. Then change one thing.” |
| “I finished a tiny fix but don't know what to do next.” | “Ask one human to try the ugly version today.” |

## Features

- 🎲 **One-button Gradio app** — type context, tap the cube.
- 🧠 **Context-aware dare engine** — no API key required for MVP.
- 🔁 **Recent-dare avoidance** — avoids repeating the same dare immediately.
- 🌈 **Cube payload** — each dare includes display text, emoji, color, and timer.
- 🔌 **ESP32-friendly API** — hardware calls one simple HTTP endpoint.
- ✅ **Tested core + API** — small enough to trust and modify.

## Quick start

```bash
git clone https://github.com/jpatel98/cube-of-tiny-dares.git
cd cube-of-tiny-dares
python3 -m pip install -r requirements.txt
python3 app.py
```

Open:

- Web UI: <http://localhost:7860>
- Health: <http://localhost:7860/api/health>

## API for hardware

The ESP32 does **not** need to know anything about Gradio.

It can call the simple JSON endpoint:

```bash
curl -sS -X POST http://localhost:7860/api/dare \
  -H 'Content-Type: application/json' \
  -d '{"context":"I keep researching and cannot pick a direction"}'
```

Example response:

```json
{
  "dare": {
    "text": "Stop researching for 20 minutes. Build the dumbest visible version.",
    "why": "More input will not pick the idea for you. A visible fake will.",
    "emoji": "🧪",
    "color": "#FFB703",
    "minutes": 20,
    "label": "research_loop"
  },
  "cube": {
    "display": "Stop researching for 20 minutes. Build the dumbest visible version.",
    "emoji": "🧪",
    "color": "#FFB703",
    "timer_seconds": 1200,
    "speak": "Stop researching for 20 minutes..."
  }
}
```

The cube only needs:

- `cube.display`
- `cube.color`
- `cube.timer_seconds`

See [`hardware/esp32_tiny_dares`](hardware/esp32_tiny_dares/) for the starter sketch.

## Hugging Face Spaces

This repo is ready for a Gradio Space.

The README contains the required Spaces metadata frontmatter, and the app entrypoint is:

```text
app.py
```

To deploy manually:

1. Create a new Hugging Face Space.
2. Select **Gradio**.
3. Push this repo to the Space.
4. The app should boot from `app.py`.

## Development

Run tests:

```bash
python3 -m pytest tests/ -q
```

Compile-check Python files:

```bash
python3 -m py_compile app.py tiny_dares/core.py
```

## Project structure

```text
app.py                         # FastAPI + Gradio app
tiny_dares/core.py             # tiny dare generator
tests/                         # pytest tests
hardware/esp32_tiny_dares/     # optional ESP32 notes/sketch
assets/social-card.svg         # repo/social preview art
plan.md                        # build plan / scope guard
```

## Scope guard

Please do **not** turn this into:

- a habit tracker
- a task manager
- a Notion integration
- a dashboard
- a full chatbot
- a wellness app

The magic is that it is almost nothing.

## Contributing

Tiny dares, hardware improvements, and vibe-preserving UX fixes are welcome.

Read [`CONTRIBUTING.md`](CONTRIBUTING.md) first.

## License

MIT — see [`LICENSE`](LICENSE).

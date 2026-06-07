# Agent Instructions

## Project Context

Cube of Tiny Dares is a Build Small Hackathon project for Hugging Face.

The app should be optimized for the Backyard AI track: solve a specific, real
problem for a builder who is stuck in loops of over-researching, over-scoping,
debugging randomly, or avoiding the next tiny visible step.

The product loop is:

```text
context -> tap -> one tiny dare -> move
```

Preserve that loop. The project should feel like a tiny physical AI appliance,
not a productivity platform.

## Hackathon Constraints

- The app must remain a Gradio app suitable for hosting as a Hugging Face Space.
- Any model dependency must follow the hackathon's small-model rule: total
  parameters must be at or below 32B.
- Prefer no cloud API dependency unless there is a clear reason and it improves
  the submission.
- Keep the README and Space metadata compatible with Hugging Face Spaces.
- Submission work should support a short demo video and social post.

## Sponsor Track Eligibility

OpenAI Codex Track:

- This project is intentionally being built with OpenAI Codex as the coding
  agent.
- Keep the public GitHub repo link visible in the Space README:
  `https://github.com/jpatel98/cube-of-tiny-dares`.
- The public repo must contain Codex-attributed commits before submission. Do
  not rewrite, squash away, or remove those commits unless Jigar explicitly asks.
- If creating new commits for submission work, prefer preserving clear Codex
  attribution in the commit history.

Modal Awards:

- The current MVP is not Modal-powered.
- Do not claim Modal eligibility unless a real Modal-backed component is added
  and documented.
- Modal usage should be load-bearing, such as an optional dare-generation
  worker, tiny model endpoint, or hardware test job. Do not add Modal only as a
  badge or empty dependency.

## Product Direction

Treat the ESP32 cube path as required for the submission, not a throwaway bonus.
The web app can be the primary demo surface, but the hardware contract should
stay real and tested:

- ESP32 button or touch input triggers one request.
- `POST /api/dare` returns simple JSON.
- Hardware only needs `cube.display`, `cube.color`, and `cube.timer_seconds`.
- Do not make the ESP32 call Gradio event endpoints directly.

The strongest identity to protect is:

```text
a tiny anti-productivity coach inside a physical cube
```

It should nudge the user into one concrete action. It should not become a chat
app, task manager, habit tracker, dashboard, Notion integration, account system,
or wellness product.

## Implementation Rules

- Make the simplest change that solves the current task.
- Do not touch unrelated files or functions.
- If intent, architecture, or requirements are unclear, ask before editing.
- If uncertain about a technical detail, state the uncertainty before
  proceeding.
- Prefer boring Python and small, focused tests.
- Keep dependencies minimal. Add one only if it directly improves the
  hackathon submission and is easy to justify.
- UI polish in `app.py` is allowed, but it must preserve the one-button cube
  experience.
- Do not add persistence, accounts, analytics, or long-term user history.
- Do not store secrets, tokens, passwords, private keys, recovery codes, or raw
  logs in the repo.

## Code Map

- `app.py` contains the FastAPI app, Gradio UI, API request model, payload
  shaping, and ASGI entrypoint.
- `tiny_dares/core.py` contains the dare bank, context classification,
  deterministic-friendly dare selection, recent-dare avoidance, and markdown
  rendering.
- `tests/test_core.py` covers core dare behavior.
- `tests/test_app.py` covers the API payload contract.
- `hardware/esp32_tiny_dares/` contains the ESP32 integration notes and starter
  Arduino sketch.
- `hardware/waveshare_tiny_dares/` contains the real Waveshare ESP32-S3 Touch
  LCD firmware adapted from AgentGotchi, including touch, display, provisioning,
  and embedded pet sprites.
- `plan.md` is the current build plan and scope guard.

## Testing

Before handing off code changes, run:

```bash
python3 -m py_compile app.py tiny_dares/core.py
python3 -m pytest tests/ -q
```

If the local `python3` is older than 3.11, use an available Python 3.11+
interpreter or create a local virtual environment. Do not commit virtualenvs,
caches, logs, or local secrets.

When changing the ESP32 contract, update both:

- `hardware/esp32_tiny_dares/README.md`
- `hardware/esp32_tiny_dares/esp32_tiny_dares.ino`

Also update or add tests for any API payload change.

## Submission Priorities

For hackathon readiness, prioritize in this order:

1. A polished Gradio app that clearly demonstrates the one-tap loop.
2. A working `/api/dare` endpoint with a stable hardware payload.
3. ESP32 integration that can trigger and display a tiny dare.
4. A short demo path that shows both web and cube behavior.
5. README/demo copy that explains why this is Backyard AI and why small is the
   point.

Avoid expanding the app just to look bigger. The strongest version is small,
specific, and demoable.

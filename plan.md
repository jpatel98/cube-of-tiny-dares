# Cube of Tiny Dares — Build Plan

## Goal

Build a tiny Gradio + ESP32-friendly hackathon project: a physical/web cube that gives one context-aware dare whenever tapped.

## Product principle

Simple, stupid, useful, cool — **not a tool**.

The loop:

1. User says what is happening.
2. User taps the cube.
3. Cube gives one tiny dare.
4. User moves.

## MVP requirements

- [x] Gradio app with a single obvious tap interaction.
- [x] Context-aware dare generator.
- [x] Hardware-friendly `/api/dare` JSON endpoint.
- [x] Recent-dare avoidance so it does not repeat immediately.
- [x] README with HF Space metadata.
- [x] ESP32 integration notes.
- [ ] Optional: deploy to Hugging Face Space.
- [ ] Optional: physical cube display/button integration.
- [ ] Optional: tiny TTS line for the dare.

## Non-goals

- No user accounts.
- No Notion.
- No task database.
- No long chat history.
- No complex hardware streaming.
- No full LLM dependency for v1.

## Demo script

1. Open app.
2. Type: `I keep researching models and can't pick a direction`.
3. Tap **TAP THE CUBE**.
4. App/cube returns: `Stop researching for 20 minutes. Build the dumbest visible version.`
5. Show `/api/dare` response for ESP32 integration.

## Hardware path

The ESP32 only needs to:

1. Connect to Wi-Fi.
2. Detect a button/touch event.
3. POST context JSON to `/api/dare`.
4. Read `cube.display`, `cube.color`, and `cube.timer_seconds`.
5. Show the dare on LCD and/or color/status.

If hardware gets annoying, submit the Gradio app first and treat ESP32 as a bonus demo.

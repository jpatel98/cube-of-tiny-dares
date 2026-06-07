# Cube of Tiny Dares — Hackathon Build Plan

## Goal

Build a tiny Gradio + ESP32 hackathon project: a physical/web cube that gives one context-aware dare whenever tapped.

This is a Hugging Face Build Small Hackathon submission for the **Backyard AI** track. The project should solve a real builder problem: getting unstuck when the next useful step is smaller than the plan in your head.

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
- [ ] Deploy to Hugging Face Space.
- [ ] Physical ESP32 cube display/button integration.
- [ ] Demo video showing web app + ESP32 cube behavior.
- [ ] Submission copy explaining the Backyard AI fit.
- [ ] Optional: tiny TTS line for the dare.

## Non-goals

- No user accounts.
- No Notion.
- No task database.
- No long chat history.
- No complex hardware streaming.
- No full chatbot behavior.
- No model larger than the hackathon's small-model limit.

## Demo script

1. Open app.
2. Type: `I keep researching models and can't pick a direction`.
3. Tap **TAP THE CUBE**.
4. App/cube returns: `Stop researching for 20 minutes. Build the dumbest visible version.`
5. Press the ESP32 cube button/touch input.
6. Show the cube calling `/api/dare` and displaying the same dare text/color/timer.

## Hardware path

The ESP32 path is required for the submission. Keep it simple and reliable.

The cube only needs to:

1. Connect to Wi-Fi.
2. Detect a button/touch event.
3. POST context JSON to `/api/dare`.
4. Read `cube.display`, `cube.color`, and `cube.timer_seconds`.
5. Show the dare on LCD and/or color/status.

Do not make the cube call Gradio event endpoints. The dedicated FastAPI endpoint is the hardware contract.

## Next build steps

1. Polish the Gradio UI for a clean one-minute demo.
2. Verify `/api/health` and `/api/dare` locally with Python 3.11.
3. Update the ESP32 sketch for the actual board, button pin, display, and Space/local endpoint.
4. Test the ESP32 against the local app.
5. Deploy to Hugging Face Space.
6. Record the demo video and prepare submission/social copy.

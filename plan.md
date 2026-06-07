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

### Hackathon readiness blockers (this sprint)

- [ ] Finalize the Space app URL and run `/api/health` + `/api/dare` against the hosted endpoint.
- [ ] Verify one physical cube trigger path that posts context and renders `cube.display`, `cube.color`, `cube.timer_seconds`.
- [ ] Produce a 30–60 second demo video that includes both web tap and cube hardware behavior.
- [ ] Finalize final submission text (identity + track alignment + limitations + one concrete next-step dare).
- [ ] Confirm the public GitHub repo contains a Codex-attributed commit for the OpenAI Codex Track.
- [ ] Keep the GitHub repo link visible in the Space README.
- [ ] Decide whether to add a real Modal-powered component; current MVP is not Modal-powered.

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

Recommended demo narration:

- "I’m over-researching and stuck. Context in, one tap, one dare, and then I can move."
- "This is the anti-overengineering loop breaker for builders, not a dashboard."

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
2. Verify `/api/health` and `/api/dare` locally with Python 3.11 and the current `/api/dare` payload fields.
3. Update the ESP32 sketch for the actual board, button pin, display, and Space/local endpoint.
4. Test the ESP32 against the local app.
5. Deploy to Hugging Face Space.
6. Record the demo video and prepare submission/social copy.

## Submission copy (ready to paste)

- **One sentence pitch:** A tiny AI appliance that turns builder analysis paralysis into one concrete action by letting you enter your loop and immediately receive one dare.
- **Track fit:** Built for Backyard AI: small, physical, specific, and built for motion not management.
- **Submission guardrails:** No accounts, no tasks, no dashboards, no wellness or SaaS surface.

## Sponsor track eligibility

- **OpenAI Codex Track:** eligible once the public GitHub repo includes Codex-attributed commits and the repo URL is included in the Space README.
- **Modal Awards:** not eligible yet. The app needs a real Modal-powered component before claiming this.

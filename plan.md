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
- [ ] Modal SLM generator for fresh dares, with local fallback.
- [ ] Optional: tiny TTS line for the dare.

### Hackathon readiness blockers (this sprint)

- [ ] Finalize the Space app URL and run `/api/health` + `/api/dare` against the hosted endpoint.
- [ ] Verify one physical cube trigger path that posts context and renders `cube.display`, `cube.color`, `cube.timer_seconds`.
- [ ] Produce a 30–60 second demo video that includes both web tap and cube hardware behavior.
- [ ] Finalize final submission text (identity + track alignment + limitations + one concrete next-step dare).
- [ ] Confirm the public GitHub repo contains a Codex-attributed commit for the OpenAI Codex Track.
- [ ] Keep the GitHub repo link visible in the Space README.
- [ ] Add and deploy the Modal SLM dare generator so Modal is a real load-bearing component.

## Non-goals

- No user accounts.
- No Notion.
- No task database.
- No long chat history.
- No complex hardware streaming.
- No full chatbot behavior.
- No model larger than the hackathon's small-model limit.
- No direct ESP32 calls to Modal; hardware only talks to `/api/dare`.

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

## Modal SLM plan

Use Modal as the primary fresh-dare generator, while keeping the local dare bank
as the reliable fallback for local development, tests, and demos.

Runtime shape:

```text
Gradio app / ESP32 -> POST /api/dare -> Modal SLM generator -> validated cube payload
                                      -> local fallback if Modal is unavailable
```

Implementation defaults:

- Default model: `Qwen/Qwen2.5-3B-Instruct`, about 3B parameters and safely under the 32B hackathon limit.
- Modal surface: a custom JSON generator endpoint, not a full OpenAI-compatible server for v1.
- App selector: `DARE_GENERATOR=auto|modal|local`, default `auto`.
- Required Modal config: `MODAL_DARE_URL`.
- Optional Modal config: `MODAL_DARE_TOKEN` if the endpoint is protected.

Modal request:

```json
{
  "context": "I keep researching models and can't pick a direction",
  "mode": "builder",
  "intensity": "medium",
  "recent": []
}
```

Modal response candidate:

```json
{
  "text": "Stop researching for 20 minutes. Build the dumbest visible version.",
  "why": "More input will not pick the idea for you. A visible fake will.",
  "emoji": "🧪",
  "color": "#FFB703",
  "minutes": 20,
  "label": "research_loop"
}
```

App-side validation remains responsible for the final hardware contract:

- one tiny visible action, not a list or chat response;
- display text short enough for the cube;
- valid hex color;
- small timer value;
- recent-dare avoidance where possible;
- fallback to local `generate_dare()` on timeout, HTTP error, bad JSON, invalid output, or missing Modal config.

The ESP32 contract does not change. The cube still reads only:

- `cube.display`
- `cube.color`
- `cube.timer_seconds`

## Next build steps

1. Add the Modal SLM generator endpoint and deploy it with `modal deploy`.
2. Wire `/api/dare` to call Modal first when configured, then fall back to the local dare bank.
3. Add tests for Modal success, Modal failure fallback, invalid output fallback, and unchanged cube payload shape.
4. Verify `/api/health` and `/api/dare` locally with Python 3.11 and the current `/api/dare` payload fields.
5. Update README submission copy to explain that Modal powers fresh SLM-generated dares.
6. Deploy to Hugging Face Space with `MODAL_DARE_URL` configured.
7. Update the ESP32 sketch endpoint for the actual Space/local URL and test the physical trigger.
8. Record the demo video and prepare submission/social copy.

## Submission copy (ready to paste)

- **One sentence pitch:** A tiny AI appliance that uses a small model on Modal to turn builder analysis paralysis into one concrete action.
- **Track fit:** Built for Backyard AI: small, physical, specific, and built for motion not management.
- **Modal fit:** Modal powers the fresh SLM-generated dare path; the local dare bank remains a fallback for reliability.
- **Submission guardrails:** No accounts, no tasks, no dashboards, no wellness or SaaS surface.

## Sponsor track eligibility

- **OpenAI Codex Track:** eligible once the public GitHub repo includes Codex-attributed commits and the repo URL is included in the Space README.
- **Modal Awards:** target eligibility by making the Modal SLM generator the primary fresh-dare path and documenting exactly what Modal powers.

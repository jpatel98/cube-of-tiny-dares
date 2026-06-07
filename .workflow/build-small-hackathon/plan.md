# Build Small Hackathon Workflow

Goal:
Prepare Cube of Tiny Dares for a Backyard AI hackathon submission.

Success criteria:
- Local app has a verified Python 3.11 setup path.
- Gradio UI is polished for a short demo while preserving one-button behavior.
- `/api/dare` hardware contract remains stable and tested.
- ESP32 docs/sketch are ready for the actual board details.
- README/plan include submission and demo copy.

Current context:
- MVP app exists in `app.py`.
- Core generator lives in `tiny_dares/core.py`.
- ESP32 starter files live in `hardware/esp32_tiny_dares/`.
- Public project direction is captured in `AGENTS.md`, `README.md`, and `plan.md`.

Constraints:
- Keep the project small.
- Do not add accounts, persistence, dashboards, analytics, or chatbot behavior.
- Keep Gradio/Hugging Face Space compatibility.
- Use only models that fit the hackathon small-model rule if adding model behavior.

Risks:
- Deployment requires external access and should be approval-gated.
- ESP32 flashing and exact display code require the actual board/display details.
- UI work should not break Gradio/FastAPI mounting or the hardware API.

Approval required:
- Installing dependencies if not already available.
- Deploying to Hugging Face Space.
- Flashing or operating hardware.
- Adding new external model/API dependencies.

Work packets:
- UI polish: improve `app.py` only, preserving API behavior.
- Docs/submission: improve README/plan/demo copy only.
- ESP32 readiness: inspect hardware files and propose/update safe generic improvements only.

Integration policy:
Workers must use disjoint write scopes and must not revert edits from other agents.

Verification:
- Run `python3 -m py_compile app.py tiny_dares/core.py`.
- Run pytest when dependencies are available.
- Smoke-test `/api/health` and `/api/dare` when the app can run.

Reusable artifacts:
- Keep this workflow under `.workflow/build-small-hackathon/` for future handoff.

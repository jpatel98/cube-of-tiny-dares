from __future__ import annotations

import os
from typing import Any

import gradio as gr
from gradio.themes import Soft
from fastapi import FastAPI
from pydantic import BaseModel, Field

from tiny_dares.core import TinyDare, generate_dare, tiny_dare_to_markdown
from tiny_dares.modal_client import ModalDareError, fetch_modal_dare, modal_configured


APP_TITLE = "Cube of Tiny Dares"
APP_TAGLINE = "Tap the cube. Get one tiny dare. Move."
DEMO_HOOK = "context → tap → one tiny dare → move"
COHERE_MODEL_ID = "CohereLabs/c4ai-command-r7b-12-2024"
LOCAL_MODEL_ID = "tiny-dares-local-rule-bank"


class DareApiRequest(BaseModel):
    context: str = Field(default="", description="What is happening right now?")
    mode: str = Field(default="builder", description="Dare mode/personality")
    intensity: str = Field(default="medium", description="gentle, medium, or spicy")
    recent: list[str] = Field(default_factory=list, description="Recently shown dare texts")
    seed: int | None = Field(default=None, description="Optional deterministic seed")


def make_cube_payload(
    dare: TinyDare,
    markdown: str,
    generation: dict[str, str | bool],
) -> dict[str, Any]:
    return {
        "dare": dare.to_dict(),
        "markdown": markdown,
        "generation": generation,
        "cube": {
            "display": dare.text,
            "emoji": dare.emoji,
            "color": dare.color,
            "timer_seconds": dare.minutes * 60,
            "speak": f"{dare.text} {dare.why}",
        },
    }


def create_dare_payload(request: DareApiRequest) -> dict[str, Any]:
    dare, generation = generate_dare_for_request(request)
    markdown = tiny_dare_to_markdown(dare)
    return make_cube_payload(dare, markdown, generation)


def generate_dare_for_request(
    request: DareApiRequest,
) -> tuple[TinyDare, dict[str, str | bool]]:
    generator = os.environ.get("DARE_GENERATOR", "modal").strip().lower()
    if generator not in {"auto", "modal", "local"}:
        generator = "modal"

    if generator in {"auto", "modal"} and modal_configured():
        try:
            dare = fetch_modal_dare(
                context=request.context,
                mode=request.mode,
                intensity=request.intensity,
                recent=request.recent,
            )
            return dare, {
                "provider": "modal",
                "model": COHERE_MODEL_ID,
                "fallback": False,
            }
        except ModalDareError:
            pass

    dare = generate_dare(
        request.context,
        mode=request.mode,
        intensity=request.intensity,
        recent=request.recent,
        seed=request.seed,
    )
    return dare, {
        "provider": "local",
        "model": LOCAL_MODEL_ID,
        "fallback": generator in {"auto", "modal"},
    }


def _cube_card(dare: TinyDare) -> str:
    return f"""
<div class="cube-card" style="border-color:{dare.color}; box-shadow: 0 0 28px {dare.color}55;">
  <div class="cube-emoji">{dare.emoji}</div>
  <div class="cube-title">{dare.text}</div>
</div>
"""


def gradio_tap(
    context: str,
    mode: str,
    intensity: str,
    recent: list[str] | None,
) -> tuple[str, list[str]]:
    recent = recent or []
    payload = create_dare_payload(
        DareApiRequest(
            context=context,
            mode=mode,
            intensity=intensity,
            recent=recent,
        )
    )
    dare = TinyDare(**payload["dare"])
    updated_recent = [dare.text, *recent][:6]
    return (
        _cube_card(dare),
        updated_recent,
    )


CSS = """
body { background: #09090f; }
.gradio-container { max-width: 980px !important; }
#app-shell {
  border: 1px solid #26263a;
  border-radius: 12px;
  padding: 18px;
  background: #11111a;
}
#hero {
  text-align: center;
  padding: 16px 4px 12px 4px;
  margin-bottom: 8px;
}
#hero h1 {
  font-size: 2.6rem;
  line-height: 1.0;
  margin-bottom: 0.25rem;
}
#hero p {
  color: #b7b7c9;
  font-size: 1.08rem;
  margin-top: 0;
}
#hero small {
  color: #8e8ea4;
  font-size: 0.92rem;
}
.cube-card {
  border: 2px solid #8338ec;
  border-radius: 8px;
  padding: 30px;
  background: radial-gradient(circle at top left, #22223b 0%, #101018 42%, #08080d 100%);
  min-height: 260px;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  text-align: center;
}
.cube-emoji { font-size: 4.4rem; margin-bottom: 14px; }
.cube-title {
  font-size: 1.95rem;
  line-height: 1.12;
  font-weight: 800;
  max-width: 720px;
}
.cube-why {
  color: #c9c9d8;
  margin-top: 14px;
  font-size: 1.05rem;
  max-width: 680px;
}
#cube-frame { border-radius: 8px; border: 1px solid #2d2d44; }
#tap-button button {
  font-size: 1.45rem;
  font-weight: 900;
  min-height: 84px;
  border-radius: 10px;
}
.small-note,
.control-note {
  color: #a3a3b8;
  font-size: 0.95rem;
  line-height: 1.3;
}
"""


def build_demo() -> gr.Blocks:
    with gr.Blocks(title=APP_TITLE) as demo:
        recent_state = gr.State([])

        with gr.Column(elem_id="app-shell"):
            gr.HTML(
                f"""
                <div id="hero">
                  <h1>🎲 {APP_TITLE}</h1>
                  <p>{APP_TAGLINE}</p>
                  <small>{DEMO_HOOK}</small>
                </div>
                """
            )

            context = gr.Textbox(
                label="What loop are you in right now?",
                placeholder="e.g. I keep researching models and can't pick a direction",
                lines=4,
            )
            with gr.Row():
                mode = gr.Dropdown(
                    choices=["builder", "hackathon", "chaos goblin", "gentle"],
                    value="builder",
                    label="Mode",
                    scale=1,
                )
                intensity = gr.Dropdown(
                    choices=["gentle", "medium", "spicy"],
                    value="medium",
                    label="Intensity",
                    scale=1,
                )
            tap = gr.Button(
                "⚡ TAP CUBE, GET ONE DARE, MOVE ⚡",
                variant="primary",
                elem_id="tap-button",
            )
            gr.Markdown(
                "<p class='control-note'>One tap = one dare. No dashboard, no accounts, no planning.</p>"
            )

            cube = gr.HTML(
                _cube_card(
                    TinyDare(
                        text="Tell me what loop you're in, then tap.",
                        why="The cube gives one tiny dare. No dashboard. No productivity cosplay.",
                        emoji="🎲",
                        color="#8338EC",
                        minutes=5,
                        label="idle",
                    )
                ),
                elem_id="cube-frame",
            )

            gr.Markdown(
                "<span class='small-note'>Hardware path: ESP32 button can POST to <code>/api/dare</code> and read <code>cube.display</code> plus <code>cube.color</code>.</span>"
            )

            gr.Markdown("#### Try these starter loops:")
            gr.Examples(
                examples=[
                    ["I keep researching models and can't pick a direction", "builder", "medium"],
                    ["I want to add login and a dashboard before the demo", "hackathon", "spicy"],
                    ["The deploy failed and I am randomly changing stuff", "builder", "medium"],
                    ["I finished a tiny fix but don't know what to do next", "gentle", "gentle"],
                ],
                inputs=[context, mode, intensity],
            )

        tap.click(
            fn=gradio_tap,
            inputs=[context, mode, intensity, recent_state],
            outputs=[cube, recent_state],
            api_name="tap",
        )

    return demo


api = FastAPI(title=APP_TITLE, version="0.1.0")


@api.get("/api/health")
def health() -> dict[str, str]:
    return {"ok": "true", "app": APP_TITLE}


@api.post("/api/dare")
def api_dare(request: DareApiRequest) -> dict[str, Any]:
    return create_dare_payload(request)


demo = build_demo()
app = gr.mount_gradio_app(api, demo, path="/", css=CSS, theme=Soft())


if __name__ == "__main__" and not os.environ.get("SPACE_ID"):
    import uvicorn

    host = os.environ.get("HOST", "0.0.0.0")
    port = int(os.environ.get("PORT", "7860"))
    uvicorn.run(app, host=host, port=port)

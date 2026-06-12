from __future__ import annotations

import os
import html
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


def _dare_output(dare: TinyDare) -> str:
    dare_text = html.escape(dare.text)
    return f"""
<div class="dare-output">
  <div class="dare-label">Your dare</div>
  <div class="dare-text">{dare_text}</div>
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
        _dare_output(dare),
        updated_recent,
    )


CSS = """
body {
  background: #111116;
}
.gradio-container {
  max-width: 760px !important;
  background: transparent !important;
}
#app-shell {
  padding: 56px 18px 36px;
  background: transparent;
}
#control-stack {
  max-width: 680px;
  margin: 0 auto;
}
#loop-input textarea {
  min-height: 86px !important;
  border-radius: 8px !important;
  border: 1px solid rgba(184, 184, 202, 0.42) !important;
  border-left-color: #8b5cf6 !important;
  background: rgba(24, 24, 34, 0.74) !important;
  color: #f7f7fb !important;
  font-size: 1.08rem !important;
  padding: 18px 20px !important;
  box-shadow: inset 0 0 0 1px rgba(255, 255, 255, 0.02);
}
#loop-input textarea::placeholder {
  color: #acacba !important;
}
#loop-input label,
#loop-input .label-wrap {
  display: none !important;
}
.example-line {
  color: #a3a3af;
  font-size: 1rem;
  margin: 16px 0 30px;
}
#tap-button {
  margin-bottom: 28px;
}
#tap-button button {
  min-height: 72px;
  border-radius: 8px;
  border: 1px solid rgba(166, 128, 255, 0.46);
  background: linear-gradient(135deg, #8b3ffc 0%, #7d34ff 46%, #6e37dc 100%);
  box-shadow: 0 14px 32px rgba(98, 55, 210, 0.28);
  color: #ffffff;
  font-size: 1.22rem;
  font-weight: 850;
  letter-spacing: 0;
}
#cube-frame {
  max-width: 680px;
  margin: 0 auto;
}
.dare-output {
  border: 1px solid rgba(255, 255, 255, 0.09);
  border-radius: 8px;
  padding: 26px 28px;
  background: rgba(16, 16, 24, 0.86);
  min-height: 138px;
}
.dare-label {
  color: #9f9faf;
  font-size: 0.92rem;
  font-weight: 700;
  margin-bottom: 14px;
  text-transform: uppercase;
}
.dare-text {
  color: #f3f3f8;
  font-size: 1.8rem;
  line-height: 1.22;
  font-weight: 750;
  overflow-wrap: anywhere;
}
@media (max-width: 720px) {
  #app-shell {
    padding: 38px 12px 28px;
  }
  .example-line {
    margin-bottom: 32px;
    font-size: 0.98rem;
  }
  #tap-button button {
    min-height: 64px;
    font-size: 1rem;
  }
  .dare-output {
    padding: 22px;
  }
  .dare-text {
    font-size: 1.42rem;
  }
}
"""


def build_demo() -> gr.Blocks:
    with gr.Blocks(title=APP_TITLE) as demo:
        recent_state = gr.State([])

        with gr.Column(elem_id="app-shell"):
            with gr.Column(elem_id="control-stack"):
                context = gr.Textbox(
                    label="What loop are you in right now?",
                    placeholder="What loop are you in right now?",
                    lines=2,
                    elem_id="loop-input",
                )
                gr.HTML(
                    '<p class="example-line">e.g. "I keep researching models and can’t pick a direction."</p>'
                )
                mode = gr.Dropdown(
                    choices=["builder", "hackathon", "chaos goblin", "gentle"],
                    value="builder",
                    label="Mode",
                    visible=False,
                )
                intensity = gr.Dropdown(
                    choices=["gentle", "medium", "spicy"],
                    value="medium",
                    label="Intensity",
                    visible=False,
                )
                tap = gr.Button(
                    "TAP CUBE, GET ONE DARE, MOVE. 🎲",
                    variant="primary",
                    elem_id="tap-button",
                )

            cube = gr.HTML("", elem_id="cube-frame")

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

from __future__ import annotations

import os
import base64
import html
from functools import lru_cache
from pathlib import Path
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
PET_AVATAR_PATH = (
    Path(__file__).parent
    / "hardware"
    / "waveshare_tiny_dares"
    / "assets"
    / "embedded"
    / "pet-happy.png"
)


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
    avatar = _pet_avatar_data_uri()
    dare_text = html.escape(dare.text)
    minutes = html.escape(str(dare.minutes))
    avatar_img = (
        f'<img src="{avatar}" alt="Tiny cube pet" class="dare-avatar-img" />'
        if avatar
        else '<span class="dare-avatar-fallback">TD</span>'
    )
    return f"""
<div class="dare-card">
  <div class="dare-avatar" style="--accent:{dare.color};">
    {avatar_img}
    <span class="dare-check">✓</span>
  </div>
  <div class="dare-body">
    <div class="dare-kicker">Here's your dare.</div>
    <div class="dare-title">{dare_text}</div>
    <div class="dare-rule"></div>
    <div class="dare-time">
      <span class="dare-time-icon">◷</span>
      <span>Do it for {minutes} minutes.</span>
    </div>
  </div>
</div>
"""


@lru_cache(maxsize=1)
def _pet_avatar_data_uri() -> str:
    if not PET_AVATAR_PATH.exists():
        return ""
    encoded = base64.b64encode(PET_AVATAR_PATH.read_bytes()).decode("ascii")
    return f"data:image/png;base64,{encoded}"


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
body {
  background:
    radial-gradient(circle at 50% 16%, rgba(116, 76, 255, 0.14), transparent 30%),
    radial-gradient(circle at 72% 58%, rgba(255, 255, 255, 0.05), transparent 24%),
    #15151d;
}
.gradio-container {
  max-width: 940px !important;
  background: transparent !important;
}
#app-shell {
  padding: 72px 18px 42px;
  background: transparent;
}
#control-stack {
  max-width: 760px;
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
  padding: 22px 24px !important;
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
  font-size: 1.08rem;
  margin: 20px 0 46px;
}
#tap-button {
  margin-bottom: 52px;
}
#tap-button button {
  min-height: 118px;
  border-radius: 8px;
  border: 1px solid rgba(166, 128, 255, 0.46);
  background: linear-gradient(135deg, #8b3ffc 0%, #7d34ff 46%, #6e37dc 100%);
  box-shadow: 0 18px 44px rgba(98, 55, 210, 0.35);
  color: #ffffff;
  font-size: 1.7rem;
  font-weight: 850;
  letter-spacing: 0;
}
#cube-frame {
  max-width: 760px;
  margin: 0 auto;
}
.dare-card {
  border: 1px solid rgba(255, 255, 255, 0.09);
  border-radius: 8px;
  padding: 30px 40px 30px;
  background: linear-gradient(145deg, rgba(17, 17, 24, 0.96), rgba(12, 12, 18, 0.84));
  min-height: 246px;
  display: flex;
  gap: 26px;
  align-items: flex-start;
  box-shadow: 0 28px 70px rgba(0, 0, 0, 0.24);
}
.dare-avatar {
  position: relative;
  flex: 0 0 72px;
  width: 72px;
  height: 72px;
  border-radius: 50%;
  display: grid;
  place-items: center;
  background: #fff8f6;
  border: 2px solid rgba(255, 255, 255, 0.78);
  overflow: visible;
}
.dare-avatar-img {
  width: 58px;
  height: 58px;
  object-fit: contain;
  image-rendering: pixelated;
}
.dare-avatar-fallback {
  color: #ff75ad;
  font-size: 0.8rem;
  font-weight: 800;
}
.dare-check {
  position: absolute;
  right: -3px;
  bottom: 7px;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  display: grid;
  place-items: center;
  background: #20aeea;
  border: 2px solid #ffffff;
  color: #ffffff;
  font-size: 0.78rem;
  font-weight: 900;
}
.dare-body {
  flex: 1;
  min-width: 0;
}
.dare-kicker {
  color: #f7f7fb;
  font-size: 1.58rem;
  line-height: 1.2;
  font-weight: 800;
  margin: 8px 0 28px;
}
.dare-title {
  color: #f3f3f8;
  font-size: 1.78rem;
  line-height: 1.35;
  font-weight: 450;
  max-width: 600px;
  overflow-wrap: anywhere;
}
.dare-rule {
  height: 1px;
  background: rgba(255, 255, 255, 0.11);
  margin: 30px 0 22px;
}
.dare-time {
  color: #a6a6b1;
  display: flex;
  gap: 12px;
  align-items: center;
  font-size: 1.18rem;
}
.dare-time-icon {
  color: #9b5cff;
  font-size: 1.4rem;
  line-height: 1;
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
    min-height: 92px;
    font-size: 1.18rem;
  }
  .dare-card {
    padding: 24px 22px;
    gap: 18px;
  }
  .dare-avatar {
    flex-basis: 58px;
    width: 58px;
    height: 58px;
  }
  .dare-avatar-img {
    width: 46px;
    height: 46px;
  }
  .dare-kicker {
    font-size: 1.25rem;
    margin-bottom: 18px;
  }
  .dare-title {
    font-size: 1.38rem;
  }
  .dare-time {
    font-size: 1rem;
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

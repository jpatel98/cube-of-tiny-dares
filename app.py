from __future__ import annotations

import os
from typing import Any

import gradio as gr
from gradio.themes import Soft
from fastapi import FastAPI
from pydantic import BaseModel, Field

from tiny_dares.core import TinyDare, generate_dare, tiny_dare_to_markdown


APP_TITLE = "Cube of Tiny Dares"
APP_TAGLINE = "Tap the cube. Get one tiny dare. Move."


class DareApiRequest(BaseModel):
    context: str = Field(default="", description="What is happening right now?")
    mode: str = Field(default="builder", description="Dare mode/personality")
    intensity: str = Field(default="medium", description="gentle, medium, or spicy")
    recent: list[str] = Field(default_factory=list, description="Recently shown dare texts")
    seed: int | None = Field(default=None, description="Optional deterministic seed")


def make_cube_payload(dare: TinyDare, markdown: str) -> dict[str, Any]:
    return {
        "dare": dare.to_dict(),
        "markdown": markdown,
        "cube": {
            "display": dare.text,
            "emoji": dare.emoji,
            "color": dare.color,
            "timer_seconds": dare.minutes * 60,
            "speak": f"{dare.text} {dare.why}",
        },
    }


def create_dare_payload(request: DareApiRequest) -> dict[str, Any]:
    dare = generate_dare(
        request.context,
        mode=request.mode,
        intensity=request.intensity,
        recent=request.recent,
        seed=request.seed,
    )
    markdown = tiny_dare_to_markdown(dare)
    return make_cube_payload(dare, markdown)


def _cube_card(dare: TinyDare) -> str:
    return f"""
<div class="cube-card" style="border-color:{dare.color}; box-shadow: 0 0 28px {dare.color}55;">
  <div class="cube-emoji">{dare.emoji}</div>
  <div class="cube-title">{dare.text}</div>
  <div class="cube-why">{dare.why}</div>
  <div class="cube-timer" style="background:{dare.color};">{dare.minutes} min dare</div>
</div>
"""


def _recent_markdown(recent: list[str]) -> str:
    if not recent:
        return "_No dares yet. Tap the cube._"
    lines = [f"{idx + 1}. {item}" for idx, item in enumerate(recent)]
    return "\n".join(lines)


def gradio_tap(
    context: str,
    mode: str,
    intensity: str,
    recent: list[str] | None,
) -> tuple[str, str, list[str], str]:
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
        payload["markdown"],
        _cube_card(dare),
        updated_recent,
        _recent_markdown(updated_recent),
    )


CSS = """
body { background: #09090f; }
.gradio-container { max-width: 980px !important; }
#hero {
  text-align: center;
  padding: 26px 18px 6px 18px;
}
#hero h1 {
  font-size: 3rem;
  line-height: 1.0;
  margin-bottom: 0.35rem;
}
#hero p {
  color: #b7b7c9;
  font-size: 1.08rem;
}
.cube-card {
  border: 2px solid #8338ec;
  border-radius: 26px;
  padding: 30px;
  background: radial-gradient(circle at top left, #22223b 0%, #101018 42%, #08080d 100%);
  min-height: 290px;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  text-align: center;
}
.cube-emoji { font-size: 4.4rem; margin-bottom: 14px; }
.cube-title {
  font-size: 2rem;
  line-height: 1.12;
  font-weight: 800;
  max-width: 720px;
}
.cube-why {
  color: #c9c9d8;
  margin-top: 16px;
  font-size: 1.05rem;
  max-width: 680px;
}
.cube-timer {
  margin-top: 22px;
  padding: 8px 14px;
  border-radius: 999px;
  color: #08080d;
  font-weight: 800;
}
#tap-button button {
  font-size: 1.35rem;
  font-weight: 900;
  min-height: 64px;
}
.small-note { color: #a3a3b8; font-size: 0.95rem; }
"""


def build_demo() -> gr.Blocks:
    with gr.Blocks(title=APP_TITLE) as demo:
        recent_state = gr.State([])

        gr.HTML(
            f"""
            <div id="hero">
              <h1>🎲 {APP_TITLE}</h1>
              <p>{APP_TAGLINE}</p>
            </div>
            """
        )

        with gr.Row():
            with gr.Column(scale=5):
                context = gr.Textbox(
                    label="What is happening right now?",
                    placeholder="e.g. I keep researching models and can't pick a direction...",
                    lines=4,
                )
                with gr.Row():
                    mode = gr.Dropdown(
                        choices=["builder", "hackathon", "chaos goblin", "gentle"],
                        value="builder",
                        label="Mode",
                    )
                    intensity = gr.Dropdown(
                        choices=["gentle", "medium", "spicy"],
                        value="medium",
                        label="Intensity",
                    )
                tap = gr.Button("TAP THE CUBE", variant="primary", elem_id="tap-button")
                gr.Markdown(
                    "<span class='small-note'>Hardware path: ESP32 button can POST to `/api/dare` and display `cube.display` + `cube.color`.</span>"
                )
            with gr.Column(scale=7):
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
                    )
                )

        with gr.Row():
            with gr.Column():
                output = gr.Markdown(label="Dare card")
            with gr.Column():
                recent_display = gr.Markdown(
                    value="_No dares yet. Tap the cube._",
                    label="Recent dares",
                )

        tap.click(
            fn=gradio_tap,
            inputs=[context, mode, intensity, recent_state],
            outputs=[output, cube, recent_state, recent_display],
            api_name="tap",
        )

        gr.Examples(
            examples=[
                ["I keep researching models and can't pick a direction", "builder", "medium"],
                ["I want to add login and a dashboard before the demo", "hackathon", "spicy"],
                ["The deploy failed and I am randomly changing stuff", "builder", "medium"],
                ["I finished a tiny fix but don't know what to do next", "gentle", "gentle"],
            ],
            inputs=[context, mode, intensity],
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


if __name__ == "__main__":
    import uvicorn

    port = int(os.environ.get("PORT", "7860"))
    uvicorn.run(app, host="0.0.0.0", port=port)

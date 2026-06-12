from __future__ import annotations

import json
import os
import re
from typing import Any

import modal
from pydantic import BaseModel, Field


MODEL_ID = "CohereLabs/c4ai-command-r7b-12-2024"
MODEL_REVISION = "main"
SYSTEM_PROMPT = """You generate one tiny dare for a builder who is stuck.
Return only compact JSON. No markdown. No extra keys.
The dare must be one concrete visible action, not advice, not a list, not a plan."""

app = modal.App("cube-of-tiny-dares-cohere")

image = (
    modal.Image.debian_slim(python_version="3.11")
    .apt_install("git")
    .uv_pip_install(
        "fastapi[standard]==0.115.6",
        "pydantic==2.10.4",
        "torch==2.5.1",
        "git+https://github.com/huggingface/transformers.git",
        "accelerate==1.2.1",
        "bitsandbytes==0.45.0",
        "sentencepiece==0.2.0",
    )
)

hf_cache = modal.Volume.from_name("cube-tiny-dares-hf-cache", create_if_missing=True)


class DareRequest(BaseModel):
    context: str = Field(default="", max_length=180)
    mode: str = Field(default="builder", max_length=24)
    intensity: str = Field(default="medium", max_length=24)
    recent: list[str] = Field(default_factory=list)


_tokenizer = None
_model = None


def _load_model():
    global _tokenizer, _model
    if _tokenizer is not None and _model is not None:
        return _tokenizer, _model

    import torch
    from transformers import AutoTokenizer, BitsAndBytesConfig, Cohere2ForCausalLM

    token = os.environ.get("HF_TOKEN")
    quantization = BitsAndBytesConfig(load_in_4bit=True)
    _tokenizer = AutoTokenizer.from_pretrained(
        MODEL_ID,
        revision=MODEL_REVISION,
        token=token,
        trust_remote_code=True,
    )
    _model = Cohere2ForCausalLM.from_pretrained(
        MODEL_ID,
        revision=MODEL_REVISION,
        token=token,
        trust_remote_code=True,
        device_map="auto",
        torch_dtype=torch.float16,
        quantization_config=quantization,
    )
    return _tokenizer, _model


def _prompt(data: DareRequest) -> str:
    recent = [item for item in data.recent[:6] if item]
    return f"""{SYSTEM_PROMPT}

Context: {data.context or "I am stuck and need a tiny dare"}
Mode: {data.mode}
Intensity: {data.intensity}
Recent dares to avoid: {recent}

Return JSON with exactly:
{{"text":"short dare under 72 chars","why":"one sentence","emoji":"one emoji","color":"#RRGGBB","minutes":10,"label":"research_loop|scope_creep|shipping|debugging|stuck|tired|demo"}}
"""


def _extract_json(text: str) -> dict[str, Any]:
    match = re.search(r"\{.*\}", text, flags=re.DOTALL)
    if not match:
        raise ValueError("no JSON object in model output")
    return json.loads(match.group(0))


@app.function(
    image=image,
    gpu="A10G",
    timeout=10 * 60,
    scaledown_window=5 * 60,
    secrets=[modal.Secret.from_name("huggingface-secret")],
    volumes={"/root/.cache/huggingface": hf_cache},
)
@modal.fastapi_endpoint(method="POST", label="dare")
def generate_dare(data: DareRequest) -> dict[str, Any]:
    tokenizer, model = _load_model()
    prompt = _prompt(data)
    messages = [{"role": "user", "content": prompt}]
    if hasattr(tokenizer, "apply_chat_template"):
        input_ids = tokenizer.apply_chat_template(
            messages,
            add_generation_prompt=True,
            return_tensors="pt",
        ).to(model.device)
    else:
        input_ids = tokenizer(prompt, return_tensors="pt").input_ids.to(model.device)

    output_ids = model.generate(
        input_ids,
        max_new_tokens=180,
        do_sample=True,
        temperature=0.7,
        top_p=0.9,
        pad_token_id=tokenizer.eos_token_id,
    )
    generated = output_ids[0][input_ids.shape[-1] :]
    text = tokenizer.decode(generated, skip_special_tokens=True)
    return _extract_json(text)

from __future__ import annotations

import json
import os
import re
from typing import Any
from urllib import error, request

from tiny_dares.core import TinyDare, classify_context


VALID_LABELS = {
    "research_loop",
    "scope_creep",
    "shipping",
    "debugging",
    "stuck",
    "tired",
    "demo",
}


class ModalDareError(RuntimeError):
    pass


def modal_configured() -> bool:
    return bool(os.environ.get("MODAL_DARE_URL", "").strip())


def fetch_modal_dare(
    *,
    context: str,
    mode: str,
    intensity: str,
    recent: list[str],
    timeout: float = 18.0,
) -> TinyDare:
    url = os.environ.get("MODAL_DARE_URL", "").strip()
    if not url:
        raise ModalDareError("MODAL_DARE_URL is not configured")

    payload = {
        "context": context,
        "mode": mode,
        "intensity": intensity,
        "recent": recent,
    }
    headers = {"Content-Type": "application/json"}
    token = os.environ.get("MODAL_DARE_TOKEN", "").strip()
    if token:
        headers["Authorization"] = f"Bearer {token}"

    body = json.dumps(payload).encode("utf-8")
    req = request.Request(url, data=body, headers=headers, method="POST")
    try:
        with request.urlopen(req, timeout=timeout) as response:
            response_body = response.read().decode("utf-8")
    except (TimeoutError, OSError, error.HTTPError, error.URLError) as exc:
        raise ModalDareError(f"Modal request failed: {exc}") from exc

    try:
        raw = json.loads(response_body)
    except json.JSONDecodeError as exc:
        raise ModalDareError("Modal returned invalid JSON") from exc

    return validate_modal_dare(raw, context=context, recent=recent)


def validate_modal_dare(raw: dict[str, Any], *, context: str, recent: list[str]) -> TinyDare:
    text = _clean_string(raw.get("text"), max_len=72)
    why = _clean_string(raw.get("why"), max_len=160)
    emoji = _clean_string(raw.get("emoji"), max_len=8) or "🎲"
    color = _clean_string(raw.get("color"), max_len=7) or "#8338EC"
    label = _clean_string(raw.get("label"), max_len=32)

    if not text:
        raise ModalDareError("Modal dare is missing text")
    if text in {item.strip() for item in recent if item.strip()}:
        raise ModalDareError("Modal repeated a recent dare")
    if not why:
        raise ModalDareError("Modal dare is missing why")
    if not re.fullmatch(r"#[0-9a-fA-F]{6}", color):
        raise ModalDareError("Modal dare color is not a hex color")
    if label not in VALID_LABELS:
        label = classify_context(context)[0]

    try:
        minutes = int(raw.get("minutes", 10))
    except (TypeError, ValueError):
        raise ModalDareError("Modal dare minutes is not an integer") from None
    if minutes < 1 or minutes > 30:
        raise ModalDareError("Modal dare minutes is outside 1-30")

    return TinyDare(
        text=text,
        why=why,
        emoji=emoji,
        color=color.upper(),
        minutes=minutes,
        label=label,
    )


def _clean_string(value: Any, *, max_len: int) -> str:
    if not isinstance(value, str):
        return ""
    cleaned = " ".join(value.strip().split())
    return cleaned[:max_len].strip()

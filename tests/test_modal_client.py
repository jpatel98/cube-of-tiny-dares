import json
import os
from unittest.mock import MagicMock, patch
from urllib import error as urllib_error

import pytest

from tiny_dares.modal_client import ModalDareError, fetch_modal_dare, validate_modal_dare


def test_validate_modal_dare_accepts_compact_output():
    dare = validate_modal_dare(
        {
            "text": "Write one demo sentence.",
            "why": "A visible claim gives the next build step a target.",
            "emoji": "⚡",
            "color": "#06d6a0",
            "minutes": 8,
            "label": "demo",
        },
        context="I am stuck on the demo",
        recent=[],
    )

    assert dare.text == "Write one demo sentence."
    assert dare.color == "#06D6A0"
    assert dare.minutes == 8
    assert dare.label == "demo"


def test_validate_modal_dare_rejects_recent_repeat():
    with pytest.raises(ModalDareError):
        validate_modal_dare(
            {
                "text": "Write one demo sentence.",
                "why": "A visible claim gives the next build step a target.",
                "emoji": "⚡",
                "color": "#06D6A0",
                "minutes": 8,
                "label": "demo",
            },
            context="I am stuck on the demo",
            recent=["Write one demo sentence."],
        )


_VALID_BASE = {
    "text": "Ship it now.",
    "why": "Momentum beats perfection.",
    "emoji": "🚀",
    "color": "#118AB2",
    "label": "shipping",
}


@pytest.mark.parametrize("minutes", [1, 30])
def test_validate_modal_dare_accepts_boundary_minutes(minutes):
    dare = validate_modal_dare(
        {**_VALID_BASE, "minutes": minutes},
        context="I want to ship",
        recent=[],
    )
    assert dare.minutes == minutes


@pytest.mark.parametrize("minutes", [0, 31])
def test_validate_modal_dare_rejects_out_of_range_minutes(minutes):
    with pytest.raises(ModalDareError, match="minutes"):
        validate_modal_dare(
            {**_VALID_BASE, "minutes": minutes},
            context="I want to ship",
            recent=[],
        )


def test_validate_modal_dare_defaults_minutes_when_key_missing():
    # When "minutes" is absent, raw.get("minutes", 10) returns 10 (a valid value).
    # The dare should be accepted with minutes=10.
    raw = {k: v for k, v in _VALID_BASE.items()}
    dare = validate_modal_dare(raw, context="I want to ship", recent=[])
    assert dare.minutes == 10


def test_fetch_modal_dare_raises_on_non_2xx(monkeypatch):
    monkeypatch.setenv("MODAL_DARE_URL", "https://example.invalid/dare")

    http_err = urllib_error.HTTPError(
        url="https://example.invalid/dare",
        code=500,
        msg="Internal Server Error",
        hdrs=None,  # type: ignore[arg-type]
        fp=None,
    )

    with patch("tiny_dares.modal_client.request.urlopen", side_effect=http_err):
        with pytest.raises(ModalDareError, match="Modal request failed"):
            fetch_modal_dare(
                context="stuck",
                mode="builder",
                intensity="medium",
                recent=[],
            )

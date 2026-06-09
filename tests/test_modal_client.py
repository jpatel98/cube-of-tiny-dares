import pytest

from tiny_dares.modal_client import ModalDareError, validate_modal_dare


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

import pytest

from tiny_dares.core import DARE_BANK, classify_context, generate_dare, tiny_dare_to_markdown


def test_classifies_research_loop_context():
    tags = classify_context("I keep researching models and can't pick a direction")

    assert "research_loop" in tags
    assert "stuck" in tags


def test_generates_short_contextual_dare_for_research_loop():
    dare = generate_dare(
        "I've been researching models all day and still can't choose an idea",
        seed=7,
    )

    assert dare.label == "research_loop"
    assert len(dare.text) <= 90
    assert dare.minutes <= 30
    assert any(
        phrase in dare.text.lower()
        for phrase in ["research", "pick", "choose", "fake", "build"]
    )
    assert dare.color.startswith("#")


def test_avoids_recent_dares_when_possible():
    first = generate_dare("I'm adding features instead of shipping", seed=3)
    second = generate_dare(
        "I'm adding features instead of shipping",
        recent=[first.text],
        seed=3,
    )

    assert second.text != first.text


def test_markdown_is_demo_friendly():
    dare = generate_dare("I shipped a tiny fix", seed=1)
    markdown = tiny_dare_to_markdown(dare)

    assert dare.text in markdown
    assert "Why" in markdown
    assert "Cube color" in markdown


def test_generate_dare_widens_when_all_label_matched_dares_are_recent():
    # Collect every dare whose label matches "research_loop" context.
    research_dares = [d for d in DARE_BANK if d.label == "research_loop"]
    # Mark all of them as recent.
    recent = [d.text for d in research_dares]

    dare = generate_dare(
        "I keep researching models",
        recent=recent,
        seed=42,
    )

    # The result must not be one of the recent dares.
    assert dare.text not in recent


def test_generate_dare_empty_string_context():
    # Empty context should not raise; should fall back to "stuck" label.
    dare = generate_dare("", seed=1)

    assert dare.text
    assert dare.label  # has some label
    assert dare.minutes >= 1

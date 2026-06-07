from tiny_dares.core import classify_context, generate_dare, tiny_dare_to_markdown


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

from __future__ import annotations

from dataclasses import asdict, dataclass
import random
import re
from typing import Iterable


@dataclass(frozen=True)
class TinyDare:
    text: str
    why: str
    emoji: str
    color: str
    minutes: int
    label: str

    def to_dict(self) -> dict[str, str | int]:
        return asdict(self)


DARE_BANK: tuple[TinyDare, ...] = (
    TinyDare(
        text="Stop researching. Build the dumbest visible version.",
        why="More input will not pick the idea for you. A visible fake will.",
        emoji="🧪",
        color="#FFB703",
        minutes=20,
        label="research_loop",
    ),
    TinyDare(
        text="Pick one option. No more comparing.",
        why="A small irreversible choice creates more signal than another comparison pass.",
        emoji="⏳",
        color="#FB8500",
        minutes=5,
        label="research_loop",
    ),
    TinyDare(
        text="Delete one feature. Keep the demo alive.",
        why="Scope creep hides as ambition. Cut until the thing has one sharp edge.",
        emoji="✂️",
        color="#EF476F",
        minutes=10,
        label="scope_creep",
    ),
    TinyDare(
        text="Ship the fake version first.",
        why="A believable demo teaches you what the real system actually needs.",
        emoji="🎭",
        color="#8338EC",
        minutes=25,
        label="scope_creep",
    ),
    TinyDare(
        text="Ask one person to try the ugly version.",
        why="A confused person is more useful than a perfect private prototype.",
        emoji="👀",
        color="#06D6A0",
        minutes=15,
        label="shipping",
    ),
    TinyDare(
        text="Write the shipped update. Make only that true.",
        why="The announcement reveals the smallest complete version.",
        emoji="📣",
        color="#118AB2",
        minutes=15,
        label="shipping",
    ),
    TinyDare(
        text="Explain the bug in one sentence.",
        why="Naming the failure usually shrinks it.",
        emoji="🦆",
        color="#FFD166",
        minutes=7,
        label="debugging",
    ),
    TinyDare(
        text="Reproduce it once. Change one thing.",
        why="Random fixes create fog. One captured failure creates a trail.",
        emoji="🔍",
        color="#3A86FF",
        minutes=12,
        label="debugging",
    ),
    TinyDare(
        text="Make one visible change before another tab.",
        why="Momentum returns through output, not browsing.",
        emoji="🧱",
        color="#8AC926",
        minutes=10,
        label="stuck",
    ),
    TinyDare(
        text="Write the next action. Do only that.",
        why="A narrow instruction beats a heroic plan.",
        emoji="🎯",
        color="#7209B7",
        minutes=10,
        label="stuck",
    ),
    TinyDare(
        text="Take a reset. Come back and remove.",
        why="Tired brains invent features to avoid finishing.",
        emoji="🌙",
        color="#577590",
        minutes=7,
        label="tired",
    ),
    TinyDare(
        text="Create the demo path. Ignore everything else.",
        why="A hackathon project is judged through the demo path first.",
        emoji="⚡",
        color="#F15BB5",
        minutes=20,
        label="demo",
    ),
)

_PATTERN_TAGS: tuple[tuple[str, str], ...] = (
    (r"research|reading|paper|papers|models?|compare|comparison|benchmark", "research_loop"),
    (r"can't pick|cannot pick|choose|direction|decide|decision|idk|unsure", "stuck"),
    (r"stuck|blocked|lost|fog|overthink|overthinking", "stuck"),
    (r"feature|scope|add|adding|auth|login|dashboard|integration", "scope_creep"),
    (r"ship|shipping|shipped|finished|done|launch|post|share", "shipping"),
    (r"bug|error|traceback|failing|failure|broken|debug|crash", "debugging"),
    (r"tired|sleepy|burnt|burned|exhausted|low energy", "tired"),
    (r"demo|hackathon|submission|deadline|video|judge|judges", "demo"),
)


def classify_context(context: str) -> list[str]:
    """Return lightweight intent tags for the user's current situation."""
    text = (context or "").strip().lower()
    tags: list[str] = []

    for pattern, tag in _PATTERN_TAGS:
        if re.search(pattern, text) and tag not in tags:
            tags.append(tag)

    if not tags:
        tags.append("stuck")

    return tags


def generate_dare(
    context: str,
    *,
    mode: str = "builder",
    intensity: str = "medium",
    recent: Iterable[str] | None = None,
    seed: int | None = None,
) -> TinyDare:
    """Generate one tiny, context-aware dare.

    The generator intentionally stays simple and deterministic-friendly so the
    core app still works without an API key or GPU. LLM seasoning can be added
    around this later, but the appliance itself should never depend on it.
    """
    del mode  # Reserved for the UI/API contract.
    del intensity  # Reserved for the UI/API contract.

    tags = classify_context(context)
    recent_set = {item.strip() for item in recent or [] if item.strip()}

    scored: list[tuple[int, TinyDare]] = []
    primary_tag = tags[0]
    for dare in DARE_BANK:
        score = 0
        if dare.label == primary_tag:
            score += 20
        if dare.label in tags:
            score += 10
        if dare.label == "stuck" and "stuck" in tags and primary_tag == "stuck":
            score += 3
        if score:
            scored.append((score, dare))

    if not scored:
        scored = [(1, dare) for dare in DARE_BANK if dare.label == "stuck"]

    available = [(score, dare) for score, dare in scored if dare.text not in recent_set]
    if available:
        scored = available

    best_score = max(score for score, _ in scored)
    best = [dare for score, dare in scored if score == best_score]
    best.sort(key=lambda item: item.text)

    rng = random.Random(seed)
    return rng.choice(best)


def tiny_dare_to_markdown(dare: TinyDare) -> str:
    return (
        f"# {dare.emoji} {dare.text}\n\n"
        f"**Why:** {dare.why}\n\n"
        f"**Cube color:** `{dare.color}`\n"
    )

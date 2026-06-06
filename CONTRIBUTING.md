# Contributing

Thanks for wanting to improve Cube of Tiny Dares.

This project is intentionally tiny. The best contributions make it **simpler, stupider, more useful, or cooler**.

## Local setup

```bash
git clone https://github.com/jpatel98/cube-of-tiny-dares.git
cd cube-of-tiny-dares
python3 -m pip install -r requirements.txt
python3 -m pytest tests/ -q
python3 app.py
```

Open <http://localhost:7860>.

## Good contributions

- New dares that create motion, not guilt.
- Better context classification.
- Cleaner ESP32 display/button examples.
- Tiny UX improvements that preserve the one-button feel.
- Better docs, screenshots, or demo scripts.

## Please avoid

- Accounts/login.
- Task management.
- Habit tracking.
- Analytics.
- Notion/database integrations.
- Full chatbot behavior.
- Complex hardware audio streaming.

If it feels like SaaS, it probably does not belong here.

## Design rule

The core loop must stay:

```text
context → tap → one tiny dare → move
```

## Tests

Run before opening a PR:

```bash
python3 -m pytest tests/ -q
python3 -m py_compile app.py tiny_dares/core.py
```

If you add generation behavior, include a test in `tests/test_core.py` or `tests/test_app.py`.

## Pull requests

1. Fork the repo.
2. Create a small branch.
3. Keep the PR focused.
4. Explain how your change helps someone move.
5. Include screenshots/API output if UI or API behavior changed.

## Code style

- Prefer boring Python.
- Avoid adding dependencies unless they remove more complexity than they add.
- Keep hardware code readable for Arduino beginners.

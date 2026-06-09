from fastapi.testclient import TestClient

import app as tiny_dares_app
from app import app
from tiny_dares.core import TinyDare
from tiny_dares.modal_client import ModalDareError


def test_api_dare_returns_cube_payload():
    client = TestClient(app)

    response = client.post(
        "/api/dare",
        json={
            "context": "I keep adding features instead of shipping the demo",
            "recent": [],
        },
    )

    assert response.status_code == 200
    payload = response.json()

    assert payload["dare"]["text"]
    assert payload["dare"]["color"].startswith("#")
    assert len(payload["cube"]["display"]) <= 72
    assert payload["cube"]["color"] == payload["dare"]["color"]
    assert "Why" in payload["markdown"]


def test_api_recent_list_changes_next_dare_when_possible():
    client = TestClient(app)

    first = client.post(
        "/api/dare",
        json={"context": "I am researching too much", "seed": 2},
    ).json()
    second = client.post(
        "/api/dare",
        json={
            "context": "I am researching too much",
            "seed": 2,
            "recent": [first["dare"]["text"]],
        },
    ).json()

    assert second["dare"]["text"] != first["dare"]["text"]


def test_api_uses_modal_generator_when_configured(monkeypatch):
    client = TestClient(app)

    monkeypatch.setenv("DARE_GENERATOR", "auto")
    monkeypatch.setenv("MODAL_DARE_URL", "https://example.invalid/dare")

    def fake_modal_dare(**kwargs):
        return TinyDare(
            text="Make the hosted demo say one true thing.",
            why="A tiny visible proof beats another setup pass.",
            emoji="⚡",
            color="#06D6A0",
            minutes=10,
            label="demo",
        )

    monkeypatch.setattr(tiny_dares_app, "fetch_modal_dare", fake_modal_dare)

    response = client.post(
        "/api/dare",
        json={"context": "I need to finish the demo"},
    )

    assert response.status_code == 200
    payload = response.json()
    assert payload["dare"]["text"] == "Make the hosted demo say one true thing."
    assert payload["cube"]["display"] == payload["dare"]["text"]


def test_api_falls_back_when_modal_fails(monkeypatch):
    client = TestClient(app)

    monkeypatch.setenv("DARE_GENERATOR", "auto")
    monkeypatch.setenv("MODAL_DARE_URL", "https://example.invalid/dare")

    def broken_modal_dare(**kwargs):
        raise ModalDareError("test failure")

    monkeypatch.setattr(tiny_dares_app, "fetch_modal_dare", broken_modal_dare)

    response = client.post(
        "/api/dare",
        json={"context": "I keep researching models", "seed": 1},
    )

    assert response.status_code == 200
    payload = response.json()
    assert payload["dare"]["text"]
    assert payload["cube"]["display"] == payload["dare"]["text"]

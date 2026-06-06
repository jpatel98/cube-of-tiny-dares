from fastapi.testclient import TestClient

from app import app


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
    assert payload["cube"]["display"] == payload["dare"]["text"]
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

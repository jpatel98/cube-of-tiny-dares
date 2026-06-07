#pragma once
#include <Arduino.h>

struct Note {
  bool   present = false;
  String text;
  String from;        // "babe" by default
  String posted_at;   // pretty relative time, e.g. "3m ago"
};

// Polls `<url>/latest` and updates `out` if a newer note is present.
// Returns true if a new note was found (caller should celebrate).
bool notesPoll(const char* url, const char* bearer, Note& out, uint32_t& last_id);

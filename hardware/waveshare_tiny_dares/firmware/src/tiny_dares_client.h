#pragma once
#include <Arduino.h>

struct TinyDarePayload {
  bool   ok = false;
  String text;
  String why;
  String emoji;
  String color;
  int    timer_seconds = 300;
  String error;
};

bool tinyDaresFetch(
  const char* endpoint,
  const char* context,
  const char* mode,
  const char* intensity,
  TinyDarePayload& out
);

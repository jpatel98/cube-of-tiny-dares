#pragma once
#include <Arduino.h>

struct AppConfig {
  String dare_url;      // Cube of Tiny Dares /api/dare endpoint
  String context;       // Default context sent by the cube
  String mode;          // builder / hackathon / gentle
  String intensity;     // gentle / medium / spicy
};

bool storageLoad(AppConfig& out);
void storageSave(const AppConfig& in);
void storageClear();

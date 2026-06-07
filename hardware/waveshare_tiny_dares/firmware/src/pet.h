#pragma once
#include <Arduino.h>

// =============================================================================
// Pet model — mood is derived from weather + time + recent events.
// Faces are text glyphs in v1; v2 will swap to pixel sprites without changing
// callers.
// =============================================================================

enum PetMood {
  MOOD_HAPPY,     // default daytime
  MOOD_EXCITED,   // just got a new note
  MOOD_BLUSHING,  // tapped within last ~2s
  MOOD_WET,       // raining outside
  MOOD_HOT,       // hot day
  MOOD_COLD,      // cold day
  MOOD_SLEEPY,    // evening
  MOOD_ASLEEP,    // quiet hours
};

struct PetContext {
  // Snapshot of the world the pet reacts to. Caller fills, pet derives mood.
  bool   weather_known    = false;
  float  temp_c           = 20.0f;
  bool   rain_now         = false;
  bool   snow_now         = false;
  int    local_hour       = 12;      // 0..23, local time
  bool   recent_tap       = false;   // tapped within last ~2s
  bool   recent_note      = false;   // note arrived within last ~5 min
};

PetMood     petMoodFor(const PetContext& c);
const char* petFace(PetMood m, uint32_t t_ms);
uint32_t    petColor(PetMood m);   // 0xRRGGBB
const char* petMoodName(PetMood m);

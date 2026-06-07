#pragma once
#include <Arduino.h>
#include "geolocation.h"

struct Weather {
  bool   fresh = false;
  uint32_t fetched_at_ms = 0;

  // Current
  float  temp_c          = 0;
  float  feels_like_c    = 0;
  int    wmo_code        = 0;      // Open-Meteo WMO weather code
  String description;              // human-readable (derived)

  // Today
  float  today_high_c    = 0;
  float  today_low_c     = 0;
  float  precip_prob_max = 0;      // % chance of any precip today
  bool   rain_today      = false;
  bool   snow_today      = false;
  String sunrise;                  // "HH:MM" local
  String sunset;                   // "HH:MM" local

  // Live state
  bool   rain_now        = false;
  bool   snow_now        = false;
};

bool weatherFetch(const GeoLocation& geo, Weather& out);

// Human-readable description for a WMO weather code.
const char* weatherDescription(int wmo);

// One-line outfit nudge based on today's forecast. Returns "" if nothing useful.
String weatherOutfitNudge(const Weather& w, const char* units /* "F" or "C" */);

// Convert °C to display value in the chosen units.
float weatherDisplayTemp(float c, const char* units);
const char* weatherTempSuffix(const char* units);  // "°F" or "°C"

#pragma once
#include <Arduino.h>

struct GeoLocation {
  bool   resolved = false;
  String city;
  String country;       // ISO-3166 alpha-2: "US", "CA", "GB", ...
  String timezone;      // "America/Toronto", "Europe/Berlin"
  float  lat = 0;
  float  lon = 0;
};

// Hits ipapi.co/json/ (no API key on free tier).
// Returns true and fills `out` on success.
bool geoFetch(GeoLocation& out);

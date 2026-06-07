#pragma once
#include <Arduino.h>

// Configure NTP and the system TZ. Call after WiFi connects.
// `tz_posix` follows POSIX TZ syntax (e.g., "EST5EDT,M3.2.0,M11.1.0").
// Pass IANA name and we'll resolve common ones; unknown → UTC.
void timeBegin(const char* iana_tz);

// Returns true if time is sync'd within reasonable bounds.
bool timeSynced();

// "HH:MM"
String timeNow();

// "Sun, May 18"
String dateNow();

// Local hour (0..23)
int    timeHourLocal();

#include "pet.h"
#include "config.h"

PetMood petMoodFor(const PetContext& c) {
  // Quiet hours always win.
  if (c.local_hour >= QUIET_HOURS_START_H || c.local_hour < QUIET_HOURS_END_H) {
    return MOOD_ASLEEP;
  }
  // Recent events take priority over ambient weather.
  if (c.recent_note) return MOOD_EXCITED;
  if (c.recent_tap)  return MOOD_BLUSHING;

  // Weather-driven moods.
  if (c.weather_known) {
    if (c.rain_now || c.snow_now) return MOOD_WET;
    if (c.temp_c > 28.0f)         return MOOD_HOT;
    if (c.temp_c < 10.0f)         return MOOD_COLD;
  }

  // Evening wind-down.
  if (c.local_hour >= 19) return MOOD_SLEEPY;

  return MOOD_HAPPY;
}

const char* petFace(PetMood m, uint32_t t) {
  switch (m) {
    case MOOD_HAPPY: {
      // Occasional blink so it feels alive.
      uint32_t phase = t % 4000;
      return (phase < 120) ? "( - . - )" : "( ^ . ^ )";
    }
    case MOOD_EXCITED:  return "( ^ o ^ )";
    case MOOD_BLUSHING: return "( > . < )";
    case MOOD_WET:      return "( o . o )";   // umbrella drawn separately
    case MOOD_HOT:      return "( - . - )";   // sunglasses drawn separately
    case MOOD_COLD:     return "( o . o )";   // scarf drawn separately
    case MOOD_SLEEPY:   return "( ~ . ~ )";
    case MOOD_ASLEEP: {
      // Gentle Zz cycle every 2s.
      uint32_t phase = t % 2000;
      return (phase < 1000) ? "( - . - ) z" : "( - . - ) Z";
    }
  }
  return "( . _ . )";
}

uint32_t petColor(PetMood m) {
  switch (m) {
    case MOOD_HAPPY:    return 0xfbcfe8;  // pink — default
    case MOOD_EXCITED:  return 0xf472b6;  // hot pink
    case MOOD_BLUSHING: return 0xfb7185;  // rose
    case MOOD_WET:      return 0x60a5fa;  // blue
    case MOOD_HOT:      return 0xfbbf24;  // amber
    case MOOD_COLD:     return 0x93c5fd;  // pale blue
    case MOOD_SLEEPY:   return 0xc4b5fd;  // lavender
    case MOOD_ASLEEP:   return 0x6b7280;  // dim grey
  }
  return 0xffffff;
}

const char* petMoodName(PetMood m) {
  switch (m) {
    case MOOD_HAPPY:    return "happy";
    case MOOD_EXCITED:  return "excited!";
    case MOOD_BLUSHING: return "blushing";
    case MOOD_WET:      return "wet";
    case MOOD_HOT:      return "hot";
    case MOOD_COLD:     return "cold";
    case MOOD_SLEEPY:   return "sleepy";
    case MOOD_ASLEEP:   return "asleep";
  }
  return "?";
}

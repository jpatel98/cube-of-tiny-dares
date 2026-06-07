#include "messages.h"
#include <esp_random.h>

// Curated fortune-cookie messages. ASCII-only because LVGL's default
// Montserrat font has no Unicode emoji or punctuation — anything outside
// 0x20-0x7E renders as a tofu rectangle on the LCD.
static const char* MESSAGES[] = {
  "you're doing amazing today <3",
  "he's thinking about you right now",
  "deep breath - you've got this",
  "drink some water, lovely",
  "later: a good thing is coming",
  "smile - someone's lucky just to know you",
  "you make ordinary days feel like magic",
  "this little cube loves you too",
  "stretch your neck, you've been hunched over",
  "save some energy for tonight",
  "soft reminder: you're allowed to rest",
  "the world is a little brighter because of you",
  "your patience today is a gift",
  "trust the version of you that's trying her best",
  "you are exactly enough",
  "tiny pet says: be kind to yourself today",
  "good things are quietly working in your favour",
  "your laugh is somebody's favourite sound",
  "p.s. you smell amazing today (probably)",
  "this is your sign to send him a meme",
};

const char* fortuneRandom() {
  size_t n = sizeof(MESSAGES) / sizeof(MESSAGES[0]);
  uint32_t r = esp_random() % n;
  return MESSAGES[r];
}

size_t fortuneCount() {
  return sizeof(MESSAGES) / sizeof(MESSAGES[0]);
}

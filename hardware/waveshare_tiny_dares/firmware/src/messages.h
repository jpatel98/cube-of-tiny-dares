#pragma once
#include <Arduino.h>

// Random sweet message from a curated bank. Used for shake-to-fortune.
const char* fortuneRandom();

// Count of available messages (for tests / cycling).
size_t fortuneCount();

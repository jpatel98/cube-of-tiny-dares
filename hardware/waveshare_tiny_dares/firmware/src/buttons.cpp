#include "buttons.h"
#include "config.h"

static bool     last_level   = true;  // pull-up high when idle
static uint32_t t_down       = 0;
static bool     hold_fired   = false;

void buttonsBegin() {
  pinMode(PIN_BTN_KEY, INPUT_PULLUP);
}

ButtonEvent buttonsPoll() {
  uint32_t now = millis();
  bool level = digitalRead(PIN_BTN_KEY);

  if (last_level && !level) {            // press edge
    t_down = now;
    hold_fired = false;
    last_level = level;
    return BTN_NONE;
  }

  if (!level && !hold_fired && (now - t_down) > 5000) {
    hold_fired = true;
    last_level = level;
    return BTN_KEY_HOLD;
  }

  if (!last_level && level) {            // release edge
    last_level = level;
    if (hold_fired) return BTN_NONE;     // hold already fired
    if ((now - t_down) < 30) return BTN_NONE;  // debounce
    return BTN_KEY_PRESS;
  }

  last_level = level;
  return BTN_NONE;
}

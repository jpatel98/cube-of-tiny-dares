#pragma once
#include <Arduino.h>

// GPIO0 (BOOT) and GPIO5 (PWR) are reserved on this board.
// We only own GPIO4 (KEY).
enum ButtonEvent {
  BTN_NONE,
  BTN_KEY_PRESS,   // short press: force immediate poll
  BTN_KEY_HOLD,    // 5s hold: factory reset
};

void buttonsBegin();
ButtonEvent buttonsPoll();

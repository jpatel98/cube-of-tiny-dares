#pragma once
#include <Arduino.h>

struct MotionState {
  bool     available = false;
  bool     moving    = false;
  bool     shake     = false;
  float    tilt_x    = 0.0f;  // -1..1, left/right board tilt
  float    tilt_y    = 0.0f;  // -1..1, forward/back board tilt
  float    spin      = 0.0f;  // -1..1, yaw impulse from the gyro
  float    energy    = 0.0f;  // 0..1, how much the board is moving
  uint32_t updated_at = 0;
};

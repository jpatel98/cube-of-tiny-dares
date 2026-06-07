#pragma once
#include <Arduino.h>
#include "motion.h"

// QMI8658 6-axis IMU. Shares the I²C bus with the touch controller.
// Polling-based motion and shake detection - no IRQ wiring needed.
bool imuBegin();

// Read the latest smoothed motion state. Safe to call every frame.
MotionState imuMotionRead();

// Should be called periodically (every ~50ms is plenty). Returns true the
// first time a shake is detected; subsequent calls return false until the
// motion settles for ~1 second (de-bounce). Kept for compatibility.
bool imuShakeDetected();

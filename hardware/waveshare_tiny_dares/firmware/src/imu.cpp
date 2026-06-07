#include "imu.h"
#include "config.h"
#include <Wire.h>
#include <math.h>
#include "SensorQMI8658.hpp"

static SensorQMI8658 qmi;
static bool ok = false;

// Shake detection parameters.
static constexpr float    SHAKE_G_THRESHOLD  = 1.8f;  // |a|/g threshold
static constexpr float    SHAKE_DPS_THRESHOLD = 260.0f;
static constexpr uint32_t SHAKE_COOLDOWN_MS  = 1200;
static uint32_t last_shake_at = 0;
static MotionState last_motion;

static float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

bool imuBegin() {
  // Wire is already begun by touchBegin(); SensorLib happily reuses it.
  // Bus pins are PIN_I2C_SDA/SCL.
  ok = qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
  if (!ok) {
    Serial.println("[imu] QMI8658 not detected");
    return false;
  }
  qmi.configAccelerometer(
      SensorQMI8658::ACC_RANGE_4G,
      SensorQMI8658::ACC_ODR_125Hz,
      SensorQMI8658::LPF_MODE_0);
  qmi.configGyroscope(
      SensorQMI8658::GYR_RANGE_512DPS,
      SensorQMI8658::GYR_ODR_112_1Hz,
      SensorQMI8658::LPF_MODE_3);
  qmi.enableGyroscope();
  qmi.enableAccelerometer();
  Serial.println("[imu] QMI8658 ready (accel+gyro)");
  return true;
}

MotionState imuMotionRead() {
  uint32_t now = millis();
  last_motion.shake = false;

  if (!ok) {
    last_motion.available = false;
    return last_motion;
  }

  if (!qmi.getDataReady()) {
    last_motion.energy *= 0.94f;
    last_motion.spin *= 0.82f;
    last_motion.moving = last_motion.energy > 0.06f;
    return last_motion;
  }

  IMUdata acc;
  IMUdata gyr;
  bool got_acc = qmi.getAccelerometer(acc.x, acc.y, acc.z);
  bool got_gyr = qmi.getGyroscope(gyr.x, gyr.y, gyr.z);
  if (!got_acc && !got_gyr) return last_motion;

  float acc_mag = 1.0f;
  if (got_acc) {
    acc_mag = sqrtf(acc.x * acc.x + acc.y * acc.y + acc.z * acc.z);
    float raw_tilt_x = clampf(acc.x / 0.75f, -1.0f, 1.0f);
    float raw_tilt_y = clampf(acc.y / 0.75f, -1.0f, 1.0f);
    last_motion.tilt_x += (raw_tilt_x - last_motion.tilt_x) * 0.22f;
    last_motion.tilt_y += (raw_tilt_y - last_motion.tilt_y) * 0.22f;
  }

  float gyro_mag = 0.0f;
  if (got_gyr) {
    gyro_mag = sqrtf(gyr.x * gyr.x + gyr.y * gyr.y + gyr.z * gyr.z);
    float raw_spin = clampf(gyr.z / 220.0f, -1.0f, 1.0f);
    last_motion.spin += (raw_spin - last_motion.spin) * 0.35f;
  } else {
    last_motion.spin *= 0.82f;
  }

  float acc_energy = got_acc ? clampf((fabsf(acc_mag - 1.0f) - 0.05f) / 0.85f, 0.0f, 1.0f) : 0.0f;
  float gyro_energy = got_gyr ? clampf((gyro_mag - 8.0f) / 220.0f, 0.0f, 1.0f) : 0.0f;
  float raw_energy = gyro_energy > acc_energy ? gyro_energy : acc_energy;
  if (raw_energy > last_motion.energy) {
    last_motion.energy += (raw_energy - last_motion.energy) * 0.48f;
  } else {
    last_motion.energy += (raw_energy - last_motion.energy) * 0.12f;
  }

  if ((acc_mag > SHAKE_G_THRESHOLD || gyro_mag > SHAKE_DPS_THRESHOLD)
      && (now - last_shake_at) > SHAKE_COOLDOWN_MS) {
    last_shake_at = now;
    last_motion.shake = true;
  }

  last_motion.available = true;
  last_motion.moving = last_motion.energy > 0.08f
                       || fabsf(last_motion.tilt_x) > 0.18f
                       || fabsf(last_motion.tilt_y) > 0.18f;
  last_motion.updated_at = now;
  return last_motion;
}

bool imuShakeDetected() {
  return imuMotionRead().shake;
}

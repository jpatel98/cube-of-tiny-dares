#include "touch.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include "TouchDrvCSTXXX.hpp"

static TouchDrvCSTXXX touch;
static volatile bool touch_irq = false;
static bool ok = false;

// Tap detection state
static bool     down = false;
static uint32_t down_at_ms = 0;
static int16_t  down_x = 0, down_y = 0;
static volatile bool tap_pending = false;

static const uint32_t TAP_MAX_MS = 400;
static const int16_t  TAP_MAX_DRAG = 12;

static void IRAM_ATTR onIrq() { touch_irq = true; }

static void touchReadLVGL(lv_indev_drv_t*, lv_indev_data_t* data);

bool touchBegin() {
  Serial.println("[touch] step 1: Wire.begin()");
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.println("[touch] step 2: setPins");
  touch.setPins(PIN_TOUCH_RST, PIN_TOUCH_IRQ);
  Serial.println("[touch] step 3: touch.begin() (I²C handshake)");
  ok = touch.begin(Wire, CST816_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.printf("[touch] step 3 returned %d\n", (int)ok);
  if (ok) {
    attachInterrupt(PIN_TOUCH_IRQ, onIrq, FALLING);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchReadLVGL;
    lv_indev_drv_register(&indev_drv);
  } else {
    Serial.println("[touch] CST816 not detected");
  }
  return ok;
}

static void touchReadLVGL(lv_indev_drv_t*, lv_indev_data_t* data) {
  if (!ok) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }

  if (touch_irq) {
    touch_irq = false;
    int16_t x[5], y[5];
    uint8_t n = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (n > 0) {
      data->state = LV_INDEV_STATE_PR;
      data->point.x = x[0];
      data->point.y = y[0];
      if (!down) {
        down = true;
        down_at_ms = millis();
        down_x = x[0];
        down_y = y[0];
      }
      return;
    }
  }

  // No active touch — handle release / tap detection.
  if (down) {
    uint32_t held = millis() - down_at_ms;
    // We don't have current x/y on release; if we had a recent down within
    // the tap window, count it as a tap.
    if (held <= TAP_MAX_MS) {
      tap_pending = true;
    }
    down = false;
  }
  data->state = LV_INDEV_STATE_REL;
}

bool touchConsumeTap() {
  if (tap_pending) {
    tap_pending = false;
    return true;
  }
  return false;
}

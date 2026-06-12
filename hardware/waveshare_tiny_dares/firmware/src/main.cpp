// =============================================================================
// Cube of Tiny Dares — Waveshare ESP32-S3 Touch LCD firmware
//
// Uses the AgentGotchi display/touch/sprite base, but the app loop is Tiny
// Dares: tap the cube, POST context to /api/dare, render one tiny dare.
// =============================================================================

#include <Arduino.h>
#include <WiFi.h>

#include "audio.h"
#include "buttons.h"
#include "config.h"
#include "display.h"
#include "imu.h"
#include "network.h"
#include "storage.h"
#include "tiny_dares_client.h"
#include "touch.h"

static AppConfig g_cfg;
static DareViewState g_view;
static uint32_t g_last_fetch_ms = 0;
static bool g_fetching = false;
static constexpr uint8_t RECENT_DARE_MAX = 6;
static String g_recent_dares[RECENT_DARE_MAX];
static uint8_t g_recent_dare_count = 0;

static void rememberDare(const String& text) {
  if (text.length() == 0) return;

  for (uint8_t i = 0; i < g_recent_dare_count; ++i) {
    if (g_recent_dares[i] == text) {
      for (uint8_t j = i; j > 0; --j) {
        g_recent_dares[j] = g_recent_dares[j - 1];
      }
      g_recent_dares[0] = text;
      return;
    }
  }

  uint8_t limit = g_recent_dare_count < RECENT_DARE_MAX ? g_recent_dare_count : RECENT_DARE_MAX - 1;
  for (uint8_t i = limit; i > 0; --i) {
    g_recent_dares[i] = g_recent_dares[i - 1];
  }
  g_recent_dares[0] = text;
  if (g_recent_dare_count < RECENT_DARE_MAX) ++g_recent_dare_count;
}

static void setIdleView() {
  g_view.status = DARE_IDLE;
  g_view.display = "Tap for one tiny dare.";
  g_view.why = g_cfg.context.length() ? g_cfg.context : DEFAULT_DARE_CONTEXT;
  g_view.color = "#8338EC";
  g_view.timer_seconds = 0;
  g_view.footer = "Touch screen or press KEY";
}

static void setLoadingView() {
  g_view.status = DARE_LOADING;
  g_view.display = "Asking the cube...";
  g_view.why = "one tiny dare, then move";
  g_view.color = "#FFB703";
  g_view.timer_seconds = 0;
  g_view.footer = "POST /api/dare";
}

static void setErrorView(const String& error) {
  g_view.status = DARE_ERROR;
  g_view.display = "Could not fetch a dare.";
  g_view.why = error.length() ? error : "check Wi-Fi and endpoint";
  g_view.color = "#EF476F";
  g_view.timer_seconds = 0;
  g_view.footer = "Tap to retry";
}

static void setReadyView(const TinyDarePayload& dare) {
  g_view.status = DARE_READY;
  g_view.display = dare.text;
  g_view.why = dare.why.length() ? dare.why : "Do this one tiny thing.";
  g_view.color = dare.color;
  g_view.timer_seconds = dare.timer_seconds;
  g_view.footer = "Tap again for another";
}

static bool ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  displayShowBoot("reconnecting Wi-Fi...");
  if (networkConnectSaved(12000)) return true;
  setErrorView("Wi-Fi disconnected");
  return false;
}

static void fetchDare() {
  uint32_t now = millis();
  if (g_fetching || now - g_last_fetch_ms < 900) return;
  g_fetching = true;
  g_last_fetch_ms = now;

  setLoadingView();
  displayTick(g_view);
  audioPlay(AUDIO_CUE_TAP);

  if (!ensureWifi()) {
    g_fetching = false;
    return;
  }

  TinyDarePayload dare;
  Serial.printf("[dare] endpoint=%s\n", g_cfg.dare_url.c_str());
  bool ok = tinyDaresFetch(
    g_cfg.dare_url.c_str(),
    g_cfg.context.c_str(),
    g_cfg.mode.c_str(),
    g_cfg.intensity.c_str(),
    g_recent_dares,
    g_recent_dare_count,
    dare
  );

  if (ok) {
    Serial.printf("[dare] %s\n", dare.text.c_str());
    rememberDare(dare.text);
    setReadyView(dare);
    audioPlay(AUDIO_CUE_NOTE);
  } else {
    Serial.printf("[dare] error: %s\n", dare.error.c_str());
    setErrorView(dare.error);
  }

  g_fetching = false;
}

static void factoryReset() {
  Serial.println("[btn] factory reset");
  displayShowProvisioning(WIFI_AP_SSID);
  storageClear();
  networkClearSaved();
  delay(300);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  uint32_t serial_start = millis();
  while (!Serial && millis() - serial_start < 1500) delay(50);
  Serial.println();
  Serial.println("=== Cube of Tiny Dares ===");

  storageLoad(g_cfg);

  if (!displayBegin()) {
    Serial.println("FATAL: display init failed");
    for (;;) delay(1000);
  }
  displayShowBoot("starting...");

  if (touchBegin()) Serial.println("[boot] touch OK");
  buttonsBegin();
  imuBegin();
  audioBegin();

  if (!networkConnectSaved()) {
    displayShowProvisioning(WIFI_AP_SSID);
    if (networkProvision(g_cfg)) {
      storageSave(g_cfg);
      delay(200);
      ESP.restart();
    }
    setErrorView("Wi-Fi setup timed out");
  } else {
    Serial.printf("[boot] Wi-Fi connected: %s\n", WiFi.localIP().toString().c_str());
    setIdleView();
  }

  Serial.printf("[boot] dare endpoint: %s\n", g_cfg.dare_url.c_str());
  displayTick(g_view);
}

void loop() {
  ButtonEvent button = buttonsPoll();
  if (button == BTN_KEY_HOLD) {
    factoryReset();
  } else if (button == BTN_KEY_PRESS) {
    fetchDare();
  }

  if (touchConsumeTap()) {
    fetchDare();
  }

  // Shake the cube as an alternate trigger for requesting a dare.
  // imuShakeDetected() already applies a 1200 ms cooldown internally, so
  // this shares the same rate-limit path as the tap/button handler above.
  if (imuShakeDetected()) {
    fetchDare();
  }

  audioTick();
  displayTick(g_view);
  delay(8);
}

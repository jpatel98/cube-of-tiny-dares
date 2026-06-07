#include "network.h"
#include "config.h"
#include <WiFi.h>
#include <WiFiManager.h>

bool networkConnectSaved(uint32_t timeout_ms) {
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout_ms) {
    delay(200);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool networkProvision(AppConfig& cfg) {
  WiFiManager wm;
  WiFiManagerParameter p_url("url", "Tiny Dares /api/dare URL",
                             cfg.dare_url.c_str(), 220);
  WiFiManagerParameter p_ctx("ctx", "Default context",
                             cfg.context.c_str(), 180);
  WiFiManagerParameter p_mode("mode", "Mode",
                              cfg.mode.c_str(), 24);
  WiFiManagerParameter p_intensity("intensity", "Intensity",
                                   cfg.intensity.c_str(), 24);

  wm.addParameter(&p_url);
  wm.addParameter(&p_ctx);
  wm.addParameter(&p_mode);
  wm.addParameter(&p_intensity);
  wm.setConfigPortalTimeout(WIFI_AP_TIMEOUT);

  if (!wm.startConfigPortal(WIFI_AP_SSID)) return false;

  cfg.dare_url  = p_url.getValue();
  cfg.context   = p_ctx.getValue();
  cfg.mode      = p_mode.getValue();
  cfg.intensity = p_intensity.getValue();
  if (cfg.dare_url.length() == 0)  cfg.dare_url = DEFAULT_DARE_ENDPOINT;
  if (cfg.context.length() == 0)   cfg.context = DEFAULT_DARE_CONTEXT;
  if (cfg.mode.length() == 0)      cfg.mode = "builder";
  if (cfg.intensity.length() == 0) cfg.intensity = "medium";
  return true;
}

void networkClearSaved() {
  WiFiManager wm;
  wm.resetSettings();
  WiFi.disconnect(true, true);
  delay(200);
}

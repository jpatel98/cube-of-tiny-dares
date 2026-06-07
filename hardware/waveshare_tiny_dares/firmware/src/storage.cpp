#include "storage.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

bool storageLoad(AppConfig& out) {
  prefs.begin(NVS_NAMESPACE, true);
  out.dare_url    = prefs.getString(NVS_KEY_DARE_URL,    DEFAULT_DARE_ENDPOINT);
  out.context     = prefs.getString(NVS_KEY_CONTEXT,     DEFAULT_DARE_CONTEXT);
  out.mode        = prefs.getString(NVS_KEY_MODE,        "builder");
  out.intensity   = prefs.getString(NVS_KEY_INTENSITY,   "medium");
  prefs.end();
  if (out.dare_url.length() == 0 ||
      out.dare_url == "http://192.168.4.2:7860/api/dare") {
    out.dare_url = DEFAULT_DARE_ENDPOINT;
  }
  // Always returns true; emptiness is meaningful but not an error.
  return true;
}

void storageSave(const AppConfig& in) {
  prefs.begin(NVS_NAMESPACE, false);
  prefs.putString(NVS_KEY_DARE_URL,    in.dare_url);
  prefs.putString(NVS_KEY_CONTEXT,     in.context);
  prefs.putString(NVS_KEY_MODE,        in.mode);
  prefs.putString(NVS_KEY_INTENSITY,   in.intensity);
  prefs.end();
}

void storageClear() {
  prefs.begin(NVS_NAMESPACE, false);
  prefs.clear();
  prefs.end();
}

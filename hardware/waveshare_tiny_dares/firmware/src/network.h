#pragma once
#include <Arduino.h>
#include "storage.h"

// Reuses saved WiFi creds. Returns true if WiFi connects within timeout.
bool networkConnectSaved(uint32_t timeout_ms = 15000);

// Blocking captive portal for first-time setup. Also collects Tiny Dares
// endpoint/context settings.
bool networkProvision(AppConfig& cfg);

// Clears saved Wi-Fi credentials so the setup portal appears on next boot.
void networkClearSaved();

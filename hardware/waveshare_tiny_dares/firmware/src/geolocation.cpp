#include "geolocation.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool geoFetch(GeoLocation& out) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();   // TLS pinning out of scope for v1
  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.setConnectTimeout(HTTP_TIMEOUT_MS);

  String url = String("https://") + GEO_API_HOST + "/json/";
  if (!http.begin(client, url)) return false;

  int code = http.GET();
  if (code != 200) { http.end(); return false; }
  String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;

  out.city     = String((const char*)(doc["city"]         | ""));
  out.country  = String((const char*)(doc["country_code"] | ""));
  out.timezone = String((const char*)(doc["timezone"]     | "UTC"));
  out.lat      = doc["latitude"]  | 0.0f;
  out.lon      = doc["longitude"] | 0.0f;
  out.resolved = out.city.length() > 0 && (out.lat != 0 || out.lon != 0);
  return out.resolved;
}

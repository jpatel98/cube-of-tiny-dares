#include "geolocation.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool geoFetch(GeoLocation& out) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  // SECURITY NOTE: certificate verification is disabled for the ipapi.co
  // geolocation call.  The only data returned (city, lat/lon, timezone,
  // country) is non-sensitive and used only for weather/pet-mood context.
  // A MITM attacker could supply a fake location, causing the weather fetch
  // to show incorrect conditions.  There is no authentication credential at
  // risk.  Full TLS verification would require embedding the ipapi.co CA
  // chain (DigiCert / GeoTrust), which adds code-size pressure and CA-rotation
  // maintenance burden for a low-value target.  Accepted risk for v1.
  client.setInsecure();
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

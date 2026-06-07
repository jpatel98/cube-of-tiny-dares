#include "notes.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

static String relTimeFor(uint32_t posted_unix) {
  time_t now = time(nullptr);
  if (now < posted_unix) return "just now";
  uint32_t diff = (uint32_t)now - posted_unix;
  if (diff < 60)      return "just now";
  if (diff < 3600)    return String(diff / 60)   + "m ago";
  if (diff < 86400)   return String(diff / 3600) + "h ago";
  return String(diff / 86400) + "d ago";
}

bool notesPoll(const char* url, const char* bearer, Note& out, uint32_t& last_id) {
  if (!url || !url[0]) return false;
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.setConnectTimeout(HTTP_TIMEOUT_MS);

  String full = String(url);
  if (!full.endsWith("/")) full += "/";
  full += "latest";
  if (!http.begin(client, full)) return false;
  if (bearer && bearer[0]) {
    http.addHeader("Authorization", String("Bearer ") + bearer);
  }

  int code = http.GET();
  if (code != 200) { http.end(); return false; }
  String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;

  uint32_t id   = doc["id"]        | 0;
  uint32_t when = doc["posted_at"] | 0;
  const char* text = doc["text"]   | "";
  const char* from = doc["from"]   | "babe";

  if (id == 0 || id == last_id) return false;   // nothing new

  out.present   = true;
  out.text      = String(text);
  out.from      = String(from);
  out.posted_at = relTimeFor(when);
  last_id       = id;
  return true;
}

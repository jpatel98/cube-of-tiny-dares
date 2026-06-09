#include "tiny_dares_client.h"
#include "config.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

static String cleanText(const char* value, const char* fallback, size_t max_len) {
  String out = (value && value[0]) ? String(value) : String(fallback);
  out.trim();
  if (out.length() == 0) out = fallback;
  if (out.length() > (int)max_len) out = out.substring(0, max_len);
  return out;
}

static bool validHexColor(const String& color) {
  if (color.length() != 7 || color[0] != '#') return false;
  for (uint8_t i = 1; i < 7; ++i) {
    char c = color[i];
    bool ok = (c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F');
    if (!ok) return false;
  }
  return true;
}

bool tinyDaresFetch(
  const char* endpoint,
  const char* context,
  const char* mode,
  const char* intensity,
  const String* recent,
  uint8_t recent_count,
  TinyDarePayload& out
) {
  out = TinyDarePayload();

  if (WiFi.status() != WL_CONNECTED) {
    out.error = "WiFi disconnected";
    return false;
  }

  String url = cleanText(endpoint, DEFAULT_DARE_ENDPOINT, 220);
  if (!url.startsWith("http://") && !url.startsWith("https://")) {
    out.error = "Bad endpoint URL";
    return false;
  }

  JsonDocument request;
  request["context"] = cleanText(context, DEFAULT_DARE_CONTEXT, 180);
  request["mode"] = cleanText(mode, "builder", 24);
  request["intensity"] = cleanText(intensity, "medium", 24);
  JsonArray recent_json = request["recent"].to<JsonArray>();
  for (uint8_t i = 0; i < recent_count; ++i) {
    if (recent[i].length() > 0) recent_json.add(recent[i]);
  }

  String body;
  serializeJson(request, body);

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.setConnectTimeout(HTTP_TIMEOUT_MS);

  WiFiClient plain_client;
  WiFiClientSecure secure_client;
  bool began = false;
  if (url.startsWith("https://")) {
    secure_client.setInsecure();
    began = http.begin(secure_client, url);
  } else {
    began = http.begin(plain_client, url);
  }

  if (!began) {
    out.error = "HTTP begin failed";
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  int status = http.POST(body);
  if (status < 200 || status >= 300) {
    out.error = String("HTTP ") + status;
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError parse_error = deserializeJson(doc, response);
  if (parse_error) {
    out.error = "Bad JSON";
    return false;
  }

  out.text = cleanText(doc["cube"]["display"] | doc["dare"]["text"] | "", "Move one tiny step.", 72);
  out.why = cleanText(doc["dare"]["why"] | "", "", 140);
  out.emoji = cleanText(doc["cube"]["emoji"] | doc["dare"]["emoji"] | "", "", 12);
  out.color = cleanText(doc["cube"]["color"] | doc["dare"]["color"] | "#8338EC", "#8338EC", 16);
  if (!validHexColor(out.color)) out.color = "#8338EC";
  out.timer_seconds = doc["cube"]["timer_seconds"] | 300;
  if (out.timer_seconds < 30) out.timer_seconds = 30;
  if (out.timer_seconds > 3600) out.timer_seconds = 3600;
  out.ok = true;
  return true;
}

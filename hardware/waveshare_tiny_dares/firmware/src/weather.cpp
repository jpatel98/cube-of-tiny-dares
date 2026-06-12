#include "weather.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WMO weather codes per Open-Meteo docs.
const char* weatherDescription(int w) {
  if (w == 0)                            return "Clear";
  if (w >= 1 && w <= 3)                  return "Partly cloudy";
  if (w == 45 || w == 48)                return "Fog";
  if (w >= 51 && w <= 57)                return "Drizzle";
  if (w >= 61 && w <= 67)                return "Rain";
  if (w >= 71 && w <= 77)                return "Snow";
  if (w >= 80 && w <= 82)                return "Showers";
  if (w == 85 || w == 86)                return "Snow showers";
  if (w >= 95 && w <= 99)                return "Thunderstorm";
  return "--";
}

static bool wmoIsRain(int w) {
  return (w >= 51 && w <= 67) || (w >= 80 && w <= 82) || (w >= 95 && w <= 99);
}
static bool wmoIsSnow(int w) {
  return (w >= 71 && w <= 77) || (w == 85) || (w == 86);
}

float weatherDisplayTemp(float c, const char* units) {
  if (units && units[0] == 'F') return c * 9.0f / 5.0f + 32.0f;
  return c;
}

const char* weatherTempSuffix(const char* units) {
  // No degree sign — not in the default Montserrat font (would tofu).
  return (units && units[0] == 'F') ? "F" : "C";
}

String weatherOutfitNudge(const Weather& w, const char* units) {
  if (!w.fresh) return "";
  if (w.snow_today)               return "bundle up - snow today";
  if (w.rain_today)               return "umbrella today";
  if (w.today_low_c < 5.0f)       return "jacket weather";
  if (w.today_high_c > 30.0f)     return "stay hydrated";
  if (w.today_high_c > 25.0f)     return "shades on, hot today";
  // Otherwise sweet nothing.
  return "hi babe <3";
}

bool weatherFetch(const GeoLocation& geo, Weather& out) {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (!geo.resolved) return false;

  WiFiClientSecure client;
  // SECURITY NOTE: certificate verification is disabled for the Open-Meteo
  // weather call.  The data (temperature, weather code, sunrise/sunset) is
  // public, non-sensitive, and used only for pet-mood display.  A MITM
  // attacker could show a fake weather condition on the screen but cannot
  // gain credentials or affect the dare endpoint.  Embedding the Open-Meteo
  // CA chain (Cloudflare / DigiCert) for a display-only feed adds maintenance
  // burden without a meaningful security benefit.  Accepted risk for v1.
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.setConnectTimeout(HTTP_TIMEOUT_MS);

  // Open-Meteo: ask for current weather + daily summary + auto-tz.
  char url[512];
  snprintf(url, sizeof(url),
    "https://%s/v1/forecast"
    "?latitude=%.4f&longitude=%.4f"
    "&current=temperature_2m,apparent_temperature,weather_code,is_day,precipitation"
    "&daily=temperature_2m_max,temperature_2m_min,weather_code,"
    "precipitation_probability_max,sunrise,sunset"
    "&timezone=%s&forecast_days=1",
    WEATHER_API_HOST, geo.lat, geo.lon, geo.timezone.c_str());

  if (!http.begin(client, url)) return false;
  int code = http.GET();
  if (code != 200) { http.end(); return false; }
  String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;

  JsonVariantConst cur = doc["current"];
  JsonVariantConst day = doc["daily"];

  out.temp_c       = cur["temperature_2m"]       | 0.0f;
  out.feels_like_c = cur["apparent_temperature"] | out.temp_c;
  out.wmo_code     = cur["weather_code"]         | 0;
  float precip_now = cur["precipitation"]        | 0.0f;
  out.description  = weatherDescription(out.wmo_code);

  out.rain_now = wmoIsRain(out.wmo_code) && precip_now > 0.01f;
  out.snow_now = wmoIsSnow(out.wmo_code) && precip_now > 0.01f;
  // Fallback: even without measurable precip in current step, if WMO says
  // active precip we let the pet know.
  if (!out.rain_now && wmoIsRain(out.wmo_code)) out.rain_now = true;
  if (!out.snow_now && wmoIsSnow(out.wmo_code)) out.snow_now = true;

  // Daily arrays — element [0] is today.
  out.today_high_c    = day["temperature_2m_max"][0]            | 0.0f;
  out.today_low_c     = day["temperature_2m_min"][0]            | 0.0f;
  out.precip_prob_max = day["precipitation_probability_max"][0] | 0.0f;
  int day_wmo         = day["weather_code"][0]                  | 0;
  out.rain_today      = wmoIsRain(day_wmo) || out.precip_prob_max > 60;
  out.snow_today      = wmoIsSnow(day_wmo);

  // "2026-05-18T07:23" → "07:23"
  String sr = String((const char*)(day["sunrise"][0] | ""));
  String ss = String((const char*)(day["sunset"][0]  | ""));
  if (sr.length() >= 16) out.sunrise = sr.substring(11, 16);
  if (ss.length() >= 16) out.sunset  = ss.substring(11, 16);

  out.fetched_at_ms = millis();
  out.fresh = true;
  return true;
}

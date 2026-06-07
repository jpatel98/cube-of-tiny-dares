#include "timekeep.h"
#include "config.h"
#include <time.h>

// Subset of IANA → POSIX TZ. Add as needed. Unknown values fall back to UTC.
static const char* posixTzFor(const char* iana) {
  if (!iana) return "UTC0";
  String s(iana);
  if (s == "America/New_York")    return "EST5EDT,M3.2.0,M11.1.0";
  if (s == "America/Toronto")     return "EST5EDT,M3.2.0,M11.1.0";
  if (s == "America/Chicago")     return "CST6CDT,M3.2.0,M11.1.0";
  if (s == "America/Denver")      return "MST7MDT,M3.2.0,M11.1.0";
  if (s == "America/Los_Angeles") return "PST8PDT,M3.2.0,M11.1.0";
  if (s == "America/Vancouver")   return "PST8PDT,M3.2.0,M11.1.0";
  if (s == "Europe/London")       return "GMT0BST,M3.5.0/1,M10.5.0";
  if (s == "Europe/Paris")        return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (s == "Europe/Berlin")       return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (s == "Asia/Kolkata")        return "IST-5:30";
  if (s == "Asia/Tokyo")          return "JST-9";
  if (s == "Asia/Shanghai")       return "CST-8";
  if (s == "Australia/Sydney")    return "AEST-10AEDT,M10.1.0,M4.1.0/3";
  return "UTC0";
}

void timeBegin(const char* iana_tz) {
  // configTzTime sets system TZ and starts SNTP in one call.
  configTzTime(posixTzFor(iana_tz), NTP_SERVER);
}

bool timeSynced() {
  time_t now = time(nullptr);
  return now > 1700000000;  // any timestamp in 2023 or later
}

String timeNow() {
  time_t now = time(nullptr);
  struct tm tm_local;
  localtime_r(&now, &tm_local);
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", tm_local.tm_hour, tm_local.tm_min);
  return String(buf);
}

String dateNow() {
  time_t now = time(nullptr);
  struct tm tm_local;
  localtime_r(&now, &tm_local);
  char buf[24];
  strftime(buf, sizeof(buf), "%a, %b %-d", &tm_local);
  return String(buf);
}

int timeHourLocal() {
  if (!timeSynced()) return 12;
  time_t now = time(nullptr);
  struct tm tm_local;
  localtime_r(&now, &tm_local);
  return tm_local.tm_hour;
}

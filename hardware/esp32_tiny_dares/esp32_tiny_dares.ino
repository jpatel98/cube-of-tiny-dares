// Cube of Tiny Dares — minimal ESP32 sketch
//
// Install dependency: ArduinoJson
// Configure WIFI_SSID, WIFI_PASSWORD, and DARE_ENDPOINT before flashing.
//
// This intentionally prints to Serial instead of assuming a specific LCD
// library. Add your Waveshare/LVGL/TFT display calls inside showDare().

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* DARE_ENDPOINT = "https://YOUR-SPACE.hf.space/api/dare";

// Change this to the actual button/touch pin for your cube.
const uint8_t BUTTON_PIN = 0;
const bool BUTTON_ACTIVE_LOW = true;

const uint16_t DEBOUNCE_MS = 750;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;
const uint16_t HTTP_TIMEOUT_MS = 8000;

constexpr const char* DEFAULT_DISPLAY = "Move one tiny step.";
constexpr const char* DEFAULT_COLOR = "#8338EC";
constexpr int DEFAULT_TIMER_SECONDS = 300;

// Keep this as a starter context; you can swap in your own app-side prompt
// source when wiring a microphone/RTC/other context input.
const char* STARTUP_CONTEXT = "I am stuck and need a tiny dare";

unsigned long lastTapMs = 0;

bool isButtonPressed() {
  return digitalRead(BUTTON_PIN) == (BUTTON_ACTIVE_LOW ? LOW : HIGH);
}

bool connectWifi() {
  WiFi.mode(WIFI_STA);
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  const unsigned long startMs = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startMs < WIFI_CONNECT_TIMEOUT_MS) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi timeout - please check credentials/network");
    return false;
  }

  Serial.print("Connected: ");
  Serial.println(WiFi.localIP());
  return true;
}

void printDarePayload(const String& text, const String& color, int timerSeconds) {
  Serial.println("--- tiny dare ---");
  Serial.print("Display: ");
  Serial.println(text);
  Serial.print("Color: ");
  Serial.println(color);
  Serial.print("Timer seconds: ");
  Serial.println(timerSeconds);
}

// Optional parser-safe trimming helper. Arduino String trim() works in-place.
String cleanTextValue(const char* rawValue, const char* fallback, size_t maxLen) {
  if (!rawValue || rawValue[0] == '\0') {
    return String(fallback);
  }
  String value(rawValue);
  value.trim();
  if (value.length() == 0) {
    return String(fallback);
  }
  if (value.length() > (int)maxLen) {
    value = value.substring(0, maxLen);
  }
  return value;
}

void showDare(const String& text, const String& color, int timerSeconds) {
  printDarePayload(text, color, timerSeconds);

  // TODO: Replace Serial output with LCD drawing for your cube.
  // Example idea:
  //   screen.fill(parseHexColor(color));
  //   screen.drawWrappedText(text);
}

void fetchDare() {
  if (WiFi.status() != WL_CONNECTED) {
    if (!connectWifi()) {
      return;
    }
  }

  if (String(DARE_ENDPOINT).startsWith("https://YOUR-SPACE")) {
    Serial.println("DARE_ENDPOINT is still placeholder. Update it before flashing.");
    return;
  }

  HTTPClient http;
  http.begin(DARE_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(HTTP_TIMEOUT_MS);

  StaticJsonDocument<384> request;
  request["context"] = STARTUP_CONTEXT;

  String body;
  serializeJson(request, body);

  int status = http.POST(body);
  Serial.print("HTTP POST status: ");
  Serial.println(status);

  if (status <= 0) {
    Serial.print("HTTP error: ");
    Serial.println(status);
    http.end();
    return;
  }
  if (status < 200 || status >= 300) {
    Serial.print("Request failed with status: ");
    Serial.println(status);
    http.end();
    return;
  }

  String response = http.getString();
  http.end();

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* display = doc["cube"]["display"] | DEFAULT_DISPLAY;
  const char* color = doc["cube"]["color"] | DEFAULT_COLOR;
  int timerSeconds = doc["cube"]["timer_seconds"] | DEFAULT_TIMER_SECONDS;

  String safeDisplay = cleanTextValue(display, DEFAULT_DISPLAY, 220);
  String safeColor = cleanTextValue(color, DEFAULT_COLOR, 16);

  showDare(safeDisplay, safeColor, timerSeconds);
}

void setup() {
  Serial.begin(115200);
  if (BUTTON_ACTIVE_LOW) {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
  } else {
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  }
  if (!connectWifi()) {
    Serial.println("Starting without Wi‑Fi. Button press will retry connection.");
  }
  showDare("Tap for a tiny dare.", DEFAULT_COLOR, 0);
}

void loop() {
  bool tapped = isButtonPressed();
  unsigned long now = millis();

  if (tapped && now - lastTapMs > DEBOUNCE_MS) {
    lastTapMs = now;
    fetchDare();
  }

  delay(20);
}

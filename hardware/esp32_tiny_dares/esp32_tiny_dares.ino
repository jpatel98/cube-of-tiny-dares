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
const int BUTTON_PIN = 0;

String currentContext = "I am stuck and need a tiny dare";
unsigned long lastTapMs = 0;
const unsigned long debounceMs = 750;

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(350);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected: ");
  Serial.println(WiFi.localIP());
}

void showDare(const String& text, const String& color, int timerSeconds) {
  Serial.println("--- tiny dare ---");
  Serial.print("Display: ");
  Serial.println(text);
  Serial.print("Color: ");
  Serial.println(color);
  Serial.print("Timer seconds: ");
  Serial.println(timerSeconds);

  // TODO: Replace Serial output with LCD drawing for your cube.
  // Example idea:
  //   screen.fill(parseHexColor(color));
  //   screen.drawWrappedText(text);
}

void fetchDare() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  HTTPClient http;
  http.begin(DARE_ENDPOINT);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<384> request;
  request["context"] = currentContext;

  String body;
  serializeJson(request, body);

  int status = http.POST(body);
  if (status <= 0) {
    Serial.print("HTTP error: ");
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

  const char* display = doc["cube"]["display"] | "Move one tiny step.";
  const char* color = doc["cube"]["color"] | "#8338EC";
  int timerSeconds = doc["cube"]["timer_seconds"] | 300;

  showDare(String(display), String(color), timerSeconds);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  connectWifi();
  showDare("Tap for a tiny dare.", "#8338EC", 0);
}

void loop() {
  bool tapped = digitalRead(BUTTON_PIN) == LOW;
  unsigned long now = millis();

  if (tapped && now - lastTapMs > debounceMs) {
    lastTapMs = now;
    fetchDare();
  }

  delay(20);
}

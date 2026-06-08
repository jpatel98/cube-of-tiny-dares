#pragma once

// =============================================================================
// Hardware pins — Waveshare ESP32-S3-Touch-LCD-1.54
// (verified against the official BSP examples)
// =============================================================================

// LCD ST7789 (SPI, 240x240)
#define PIN_LCD_DC      45
#define PIN_LCD_CS      21
#define PIN_LCD_SCK     38
#define PIN_LCD_MOSI    39
#define PIN_LCD_MISO    -1
#define PIN_LCD_RST     40
#define PIN_LCD_BL      46
#define LCD_WIDTH       240
#define LCD_HEIGHT      240

// CST816 capacitive touch + QMI8658 IMU share the same I²C bus
#define PIN_I2C_SDA     42
#define PIN_I2C_SCL     41
#define PIN_TOUCH_IRQ   48
#define PIN_TOUCH_RST   47

// Single safe user button (BOOT and PWR are reserved on this board)
#define PIN_BTN_KEY     4

// ES8311 audio codec + speaker amp (I2S, Waveshare BSP pinout)
#define PIN_AUDIO_PA_CTRL   7
#define PIN_AUDIO_MCLK      8
#define PIN_AUDIO_BCLK      9
#define PIN_AUDIO_DOUT      12
#define PIN_AUDIO_LRC       10

// =============================================================================
// Network endpoints
// =============================================================================

// Default Tiny Dares API endpoint. Override this in the captive portal after
// Wi-Fi provisioning, or point it at the hosted Hugging Face Space.
#define DEFAULT_DARE_ENDPOINT  "https://build-small-hackathon-cube-of-tiny-dares.hf.space/api/dare"
#define DEFAULT_DARE_CONTEXT   "I am stuck and need a tiny dare"

// Open-Meteo — weather. No API key, no rate limit hassle for low-poll use.
// Docs: https://open-meteo.com/en/docs
#define WEATHER_API_HOST     "api.open-meteo.com"

// ipapi.co — IP-based geolocation (city / lat,lon / timezone / country).
// Free tier: 1000 req/day. We hit it once at boot and every ~6h after.
// Docs: https://ipapi.co/api/
#define GEO_API_HOST         "ipapi.co"

// NTP
#define NTP_SERVER           "pool.ntp.org"

// =============================================================================
// Polling intervals
// =============================================================================
#define WEATHER_POLL_MS         (10 * 60 * 1000UL)   // every 10 min
#define GEO_POLL_MS             (6  * 60 * 60 * 1000UL) // every 6 h
#define NOTES_POLL_MS           (30 * 1000UL)        // every 30 s
#define NTP_RESYNC_MS           (6  * 60 * 60 * 1000UL) // every 6 h
#define HTTP_TIMEOUT_MS         8000

// =============================================================================
// NVS keys (persistent config)
// =============================================================================
#define NVS_NAMESPACE   "agentgotchi"
#define NVS_KEY_DARE_URL    "dare_url"      // Cube of Tiny Dares /api/dare URL
#define NVS_KEY_CONTEXT     "context"       // Default context sent by the cube
#define NVS_KEY_MODE        "mode"          // builder / hackathon / gentle
#define NVS_KEY_INTENSITY   "intensity"     // gentle / medium / spicy

// Legacy AgentGotchi keys kept so old modules still compile if reused later.
#define NVS_KEY_NOTES_URL   "notes_url"
#define NVS_KEY_NOTES_TOKEN "notes_token"
#define NVS_KEY_NAME        "her_name"
#define NVS_KEY_UNITS       "units"

// =============================================================================
// Provisioning
// =============================================================================
#define WIFI_AP_SSID    "TinyDares-setup"
#define WIFI_AP_TIMEOUT 300  // seconds

// =============================================================================
// UI behaviour
// =============================================================================
#define QUIET_HOURS_START_H  21   // 9 PM — pet starts winding down
#define QUIET_HOURS_END_H    7    // 7 AM — pet wakes
#define VIEW_AUTO_RETURN_MS  20000 // 20s of no input → snap back to HOME

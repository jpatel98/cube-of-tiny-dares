#include "display.h"
#include "config.h"
#include "assets/agentgotchi_assets.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

static Arduino_DataBus* bus = nullptr;
static Arduino_GFX* gfx = nullptr;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t* disp_buf = nullptr;
static bool sleeping = false;

static lv_obj_t* scr_main = nullptr;
static lv_obj_t* pet_img = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* dare_label = nullptr;
static lv_obj_t* detail_label = nullptr;
static lv_obj_t* panel = nullptr;

static uint32_t last_render_hash = 0;

static void flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);
#endif
  lv_disp_flush_ready(drv);
}

static uint32_t colorFromHex(const String& value, uint32_t fallback) {
  if (value.length() != 7 || value[0] != '#') return fallback;
  char buf[7];
  for (uint8_t i = 0; i < 6; ++i) buf[i] = value[i + 1];
  buf[6] = '\0';
  char* end = nullptr;
  uint32_t parsed = strtoul(buf, &end, 16);
  return (end && *end == '\0') ? parsed : fallback;
}

static const lv_img_dsc_t* petForState(DareViewStatus status) {
  (void)status;
  return &agentgotchi_pet_happy;
}

static const char* titleForStatus(DareViewStatus status) {
  switch (status) {
    case DARE_LOADING: return "asking cube...";
    case DARE_READY:   return "tiny dare";
    case DARE_ERROR:   return "try again";
    case DARE_IDLE:
    default:           return "cube of tiny dares";
  }
}

static void setLabel(lv_obj_t* label, const String& text) {
  lv_label_set_text(label, text.c_str());
}

static void buildMainScreen() {
  scr_main = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(scr_main, lv_color_hex(0x11111A), 0);
  lv_obj_set_style_bg_opa(scr_main, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(scr_main, 0, 0);
  lv_obj_clear_flag(scr_main, LV_OBJ_FLAG_SCROLLABLE);

  panel = lv_obj_create(scr_main);
  lv_obj_set_size(panel, LCD_WIDTH - 16, LCD_HEIGHT - 16);
  lv_obj_set_pos(panel, 8, 8);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x151821), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(panel, 10, 0);
  lv_obj_set_style_border_width(panel, 2, 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x3A2D44), 0);
  lv_obj_set_style_pad_all(panel, 0, 0);
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

  title_label = lv_label_create(panel);
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(title_label, lv_color_hex(0xE8E8F2), 0);
  lv_obj_set_width(title_label, LCD_WIDTH - 32);
  lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 12);

  pet_img = lv_img_create(panel);
  lv_img_set_src(pet_img, &agentgotchi_pet_happy);
  lv_img_set_pivot(pet_img, 56, 56);
  lv_img_set_zoom(pet_img, 212);
  lv_obj_align(pet_img, LV_ALIGN_TOP_MID, 0, 34);

  dare_label = lv_label_create(panel);
  lv_obj_set_style_text_font(dare_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(dare_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_align(dare_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_line_space(dare_label, 5, 0);
  lv_obj_set_width(dare_label, LCD_WIDTH - 38);
  lv_label_set_long_mode(dare_label, LV_LABEL_LONG_WRAP);
  lv_obj_align(dare_label, LV_ALIGN_TOP_MID, 0, 140);

  detail_label = lv_label_create(panel);
  lv_obj_set_style_text_font(detail_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(detail_label, lv_color_hex(0xA3A3B8), 0);
  lv_obj_set_style_text_align(detail_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(detail_label, LCD_WIDTH - 42);
  lv_label_set_long_mode(detail_label, LV_LABEL_LONG_DOT);
  lv_obj_align(detail_label, LV_ALIGN_BOTTOM_MID, 0, -14);
}

bool displayBegin() {
  bus = new Arduino_ESP32SPI(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_MOSI, PIN_LCD_MISO);
  gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT);
  if (!gfx->begin()) {
    Serial.println("[display] gfx->begin() failed");
    return false;
  }
  gfx->fillScreen(BLACK);
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);

  lv_init();

  const size_t buf_lines = 40;
  size_t buf_px = LCD_WIDTH * buf_lines;
  disp_buf = (lv_color_t*)heap_caps_malloc(buf_px * sizeof(lv_color_t),
                                           MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_buf) disp_buf = (lv_color_t*)heap_caps_malloc(buf_px * sizeof(lv_color_t), MALLOC_CAP_8BIT);
  if (!disp_buf) {
    Serial.println("[display] draw buf alloc failed");
    return false;
  }
  lv_disp_draw_buf_init(&draw_buf, disp_buf, nullptr, buf_px);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_WIDTH;
  disp_drv.ver_res = LCD_HEIGHT;
  disp_drv.flush_cb = flush_cb;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  buildMainScreen();
  lv_scr_load(scr_main);
  return true;
}

void displaySleep(bool sleep) {
  sleeping = sleep;
  digitalWrite(PIN_LCD_BL, sleep ? LOW : HIGH);
}

void displayShowBoot(const char* msg) {
  DareViewState s;
  s.status = DARE_IDLE;
  s.display = "waking up...";
  s.why = msg ? msg : "";
  displayTick(s);
}

void displayShowProvisioning(const char* ssid) {
  DareViewState s;
  s.status = DARE_ERROR;
  s.display = "Wi-Fi setup";
  s.why = ssid ? String("Join ") + ssid : "Open setup portal";
  displayTick(s);
}

void displayTick(const DareViewState& s) {
  if (sleeping) {
    lv_timer_handler();
    return;
  }

  uint32_t accent = colorFromHex(s.color, 0x8338EC);
  uint32_t hash = (uint32_t)s.status ^
                  (accent << 1) ^
                  (uint32_t)s.display.length() << 8 ^
                  (uint32_t)s.why.length() << 16;

  lv_obj_set_style_border_color(panel, lv_color_hex(accent), 0);

  const lv_img_dsc_t* pet = petForState(s.status);
  lv_img_set_src(pet_img, pet);

  uint32_t t = millis();
  int16_t bob = (s.status == DARE_LOADING) ? ((t / 180) % 2 ? -4 : 2) : 0;
  int16_t zoom = (s.status == DARE_READY) ? 216 : 208;
  lv_img_set_zoom(pet_img, zoom);
  lv_obj_align(pet_img, LV_ALIGN_TOP_MID, 0, 34 + bob);

  if (hash != last_render_hash) {
    lv_label_set_text(title_label, titleForStatus(s.status));
    setLabel(dare_label, s.display);
    if (s.status == DARE_READY) {
      lv_obj_add_flag(detail_label, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(detail_label, LV_OBJ_FLAG_HIDDEN);
      setLabel(detail_label, s.why);
    }
    last_render_hash = hash;
  }

  lv_timer_handler();
}

#include "audio.h"

#include <Wire.h>
#include <driver/i2s.h>
#include <math.h>

#include "config.h"

namespace {

static constexpr i2s_port_t AUDIO_I2S_PORT = I2S_NUM_0;
static constexpr uint32_t AUDIO_SAMPLE_RATE = 16000;
static constexpr uint32_t AUDIO_MCLK_HZ = AUDIO_SAMPLE_RATE * 256;
static constexpr uint8_t ES8311_ADDR = 0x18;
static constexpr size_t AUDIO_CHUNK_FRAMES = 256;
static constexpr float AUDIO_TWO_PI = 6.2831853071795864769f;

struct AudioSegment {
  uint16_t freq_hz;
  uint16_t duration_ms;
  float gain;
};

static const AudioSegment TAP_CUE[] = {
  {880, 34, 0.30f},
  {1319, 46, 0.24f},
};

static const AudioSegment NOTE_CUE[] = {
  {659, 70, 0.26f},
  {880, 70, 0.24f},
  {1175, 110, 0.20f},
};

static const AudioSegment FORTUNE_CUE[] = {
  {1047, 45, 0.22f},
  {1568, 55, 0.20f},
  {2093, 75, 0.17f},
  {0, 30, 0.0f},
  {1760, 70, 0.15f},
};

struct CueDef {
  const AudioSegment* segments;
  uint8_t count;
};

static bool ready = false;
static bool muted = false;
static bool playing = false;
static const CueDef* activeCue = nullptr;
static uint8_t activeSegment = 0;
static uint32_t segmentSample = 0;
static float phase = 0.0f;
static bool loggedUnavailable = false;

static uint32_t minU32(uint32_t a, uint32_t b) {
  return a < b ? a : b;
}

static uint32_t maxU32(uint32_t a, uint32_t b) {
  return a > b ? a : b;
}

static bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission(true) == 0;
}

static bool readReg(uint8_t reg, uint8_t& value) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint16_t)ES8311_ADDR, (uint8_t)1, (uint8_t)true) != 1) {
    return false;
  }
  value = Wire.read();
  return true;
}

static bool probeCodec() {
  Wire.beginTransmission(ES8311_ADDR);
  return Wire.endTransmission(true) == 0;
}

static bool configureEs8311() {
  if (!probeCodec()) {
    Serial.println("[audio] ES8311 not detected at 0x18; audio disabled");
    return false;
  }

  // Minimal 16 kHz / 16-bit DAC path, adapted from the Waveshare ES8311 setup.
  if (!writeReg(0x00, 0x1F)) return false;  // Reset digital, clock manager, etc.
  delay(20);
  if (!writeReg(0x00, 0x00)) return false;
  if (!writeReg(0x00, 0x80)) return false;  // Power-on command.
  if (!writeReg(0x01, 0x3F)) return false;  // Enable all clocks, MCLK from MCLK pin.

  uint8_t reg = 0;
  if (!readReg(0x06, reg)) return false;
  reg &= ~(1 << 5);  // BCLK not inverted.
  if (!writeReg(0x06, reg)) return false;

  // Coefficients for MCLK=4.096 MHz and Fs=16 kHz from the ES8311 table.
  if (!readReg(0x02, reg)) return false;
  reg &= 0x07;
  reg |= (uint8_t)((1 - 1) << 5);  // pre_div
  reg |= (uint8_t)(0 << 3);        // pre_multi
  if (!writeReg(0x02, reg)) return false;
  if (!writeReg(0x03, 0x10)) return false;  // fs_mode + ADC OSR.
  if (!writeReg(0x04, 0x10)) return false;  // DAC OSR.
  if (!writeReg(0x05, 0x00)) return false;  // ADC/DAC dividers.

  if (!readReg(0x06, reg)) return false;
  reg &= 0xE0;
  reg |= 0x03;  // BCLK divider 4 -> register value 3.
  if (!writeReg(0x06, reg)) return false;

  if (!readReg(0x07, reg)) return false;
  reg &= 0xC0;
  if (!writeReg(0x07, reg)) return false;
  if (!writeReg(0x08, 0xFF)) return false;  // LRCK low byte.

  // Slave I2S format, 16-bit input/output.
  if (!readReg(0x00, reg)) return false;
  reg &= 0xBF;
  if (!writeReg(0x00, reg)) return false;
  if (!writeReg(0x09, (uint8_t)(3 << 2))) return false;
  if (!writeReg(0x0A, (uint8_t)(3 << 2))) return false;

  if (!writeReg(0x0D, 0x01)) return false;  // Power up analog circuitry.
  if (!writeReg(0x0E, 0x02)) return false;  // Enable analog PGA and ADC modulator.
  if (!writeReg(0x12, 0x00)) return false;  // Power up DAC.
  if (!writeReg(0x13, 0x10)) return false;  // Enable output to headphone driver.
  if (!writeReg(0x1C, 0x6A)) return false;  // ADC EQ bypass / DC cancellation.
  if (!writeReg(0x37, 0x08)) return false;  // DAC EQ bypass.
  if (!writeReg(0x17, 0xC8)) return false;  // ADC volume per reference setup.
  if (!writeReg(0x14, 0x1A)) return false;  // Analog mic path, digital mic disabled.
  if (!writeReg(0x32, 0xBF)) return false;  // DAC volume: about 75%.

  if (!readReg(0x31, reg)) return false;
  reg &= ~((1 << 6) | (1 << 5));
  if (!writeReg(0x31, reg)) return false;

  return true;
}

static bool configureI2s() {
  i2s_config_t i2s_config = {};
  i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  i2s_config.sample_rate = AUDIO_SAMPLE_RATE;
  i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  i2s_config.intr_alloc_flags = 0;
  i2s_config.dma_buf_count = 4;
  i2s_config.dma_buf_len = AUDIO_CHUNK_FRAMES;
  i2s_config.use_apll = false;
  i2s_config.tx_desc_auto_clear = true;
  i2s_config.fixed_mclk = AUDIO_MCLK_HZ;
  i2s_config.mclk_multiple = I2S_MCLK_MULTIPLE_256;
  i2s_config.bits_per_chan = I2S_BITS_PER_CHAN_16BIT;

  esp_err_t err = i2s_driver_install(AUDIO_I2S_PORT, &i2s_config, 0, nullptr);
  if (err != ESP_OK) {
    Serial.printf("[audio] i2s_driver_install failed: %d\n", (int)err);
    return false;
  }

  i2s_pin_config_t pin_config = {};
  pin_config.mck_io_num = PIN_AUDIO_MCLK;
  pin_config.bck_io_num = PIN_AUDIO_BCLK;
  pin_config.ws_io_num = PIN_AUDIO_LRC;
  pin_config.data_out_num = PIN_AUDIO_DOUT;
  pin_config.data_in_num = I2S_PIN_NO_CHANGE;

  err = i2s_set_pin(AUDIO_I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[audio] i2s_set_pin failed: %d\n", (int)err);
    i2s_driver_uninstall(AUDIO_I2S_PORT);
    return false;
  }

  i2s_zero_dma_buffer(AUDIO_I2S_PORT);
  return true;
}

static const CueDef* cueFor(AudioCue cue) {
  static const CueDef tap = {TAP_CUE, (uint8_t)(sizeof(TAP_CUE) / sizeof(TAP_CUE[0]))};
  static const CueDef note = {NOTE_CUE, (uint8_t)(sizeof(NOTE_CUE) / sizeof(NOTE_CUE[0]))};
  static const CueDef fortune = {FORTUNE_CUE, (uint8_t)(sizeof(FORTUNE_CUE) / sizeof(FORTUNE_CUE[0]))};

  switch (cue) {
    case AUDIO_CUE_TAP: return &tap;
    case AUDIO_CUE_NOTE: return &note;
    case AUDIO_CUE_FORTUNE: return &fortune;
  }
  return nullptr;
}

static void advanceSegment() {
  segmentSample = 0;
  phase = 0.0f;
  activeSegment++;
  if (!activeCue || activeSegment >= activeCue->count) {
    playing = false;
    activeCue = nullptr;
  }
}

static int16_t nextSample() {
  if (!activeCue || activeSegment >= activeCue->count) {
    playing = false;
    return 0;
  }

  const AudioSegment& seg = activeCue->segments[activeSegment];
  const uint32_t totalSamples =
      maxU32(1, (AUDIO_SAMPLE_RATE * (uint32_t)seg.duration_ms) / 1000U);

  if (segmentSample >= totalSamples) {
    advanceSegment();
    return nextSample();
  }

  float env = 1.0f;
  const uint32_t rampSamples = minU32(80, totalSamples / 2);
  if (rampSamples > 0) {
    if (segmentSample < rampSamples) {
      env = (float)segmentSample / (float)rampSamples;
    } else {
      const uint32_t remaining = totalSamples - segmentSample;
      if (remaining < rampSamples) env = (float)remaining / (float)rampSamples;
    }
  }

  int16_t sample = 0;
  if (seg.freq_hz > 0 && seg.gain > 0.0f) {
    sample = (int16_t)(sinf(phase) * (32767.0f * seg.gain * env));
    phase += AUDIO_TWO_PI * (float)seg.freq_hz / (float)AUDIO_SAMPLE_RATE;
    if (phase >= AUDIO_TWO_PI) phase -= AUDIO_TWO_PI;
  }

  segmentSample++;
  if (segmentSample >= totalSamples) advanceSegment();
  return sample;
}

}  // namespace

bool audioBegin() {
  if (ready) return true;

  pinMode(PIN_AUDIO_PA_CTRL, OUTPUT);
  digitalWrite(PIN_AUDIO_PA_CTRL, LOW);

  if (!configureI2s()) {
    loggedUnavailable = true;
    return false;
  }

  if (!configureEs8311()) {
    Serial.println("[audio] ES8311 init failed; audio disabled");
    i2s_driver_uninstall(AUDIO_I2S_PORT);
    loggedUnavailable = true;
    return false;
  }

  digitalWrite(PIN_AUDIO_PA_CTRL, HIGH);
  ready = true;
  Serial.printf("[audio] ready: ES8311, I2S %lu Hz on BCLK=%d LRC=%d DOUT=%d MCLK=%d\n",
                (unsigned long)AUDIO_SAMPLE_RATE,
                PIN_AUDIO_BCLK,
                PIN_AUDIO_LRC,
                PIN_AUDIO_DOUT,
                PIN_AUDIO_MCLK);
  return true;
}

void audioTick() {
  if (!ready || muted || !playing || !activeCue) return;

  int16_t frames[AUDIO_CHUNK_FRAMES * 2];
  size_t frameCount = 0;
  while (frameCount < AUDIO_CHUNK_FRAMES && playing) {
    const int16_t sample = nextSample();
    frames[frameCount * 2] = sample;
    frames[frameCount * 2 + 1] = sample;
    frameCount++;
  }

  if (frameCount == 0) return;

  size_t bytesWritten = 0;
  const size_t byteCount = frameCount * 2 * sizeof(int16_t);
  i2s_write(AUDIO_I2S_PORT, frames, byteCount, &bytesWritten, 0);
}

bool audioPlay(AudioCue cue) {
  if (!ready || muted) {
    if (!ready && !loggedUnavailable) {
      Serial.println("[audio] unavailable; cue ignored");
      loggedUnavailable = true;
    }
    return false;
  }

  const CueDef* cueDef = cueFor(cue);
  if (!cueDef) return false;

  activeCue = cueDef;
  activeSegment = 0;
  segmentSample = 0;
  phase = 0.0f;
  playing = true;
  return true;
}

void audioSetMuted(bool value) {
  muted = value;
  if (ready) {
    digitalWrite(PIN_AUDIO_PA_CTRL, muted ? LOW : HIGH);
    uint8_t reg = 0;
    if (readReg(0x31, reg)) {
      if (muted) {
        reg |= (1 << 6) | (1 << 5);
      } else {
        reg &= ~((1 << 6) | (1 << 5));
      }
      writeReg(0x31, reg);
    }
    if (muted) {
      playing = false;
      activeCue = nullptr;
      i2s_zero_dma_buffer(AUDIO_I2S_PORT);
    }
  }
}

bool audioMuted() {
  return muted;
}

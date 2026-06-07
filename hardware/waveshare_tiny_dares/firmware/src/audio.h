#pragma once
#include <Arduino.h>

enum AudioCue {
  AUDIO_CUE_TAP,
  AUDIO_CUE_NOTE,
  AUDIO_CUE_FORTUNE,
};

bool audioBegin();
void audioTick();
bool audioPlay(AudioCue cue);
void audioSetMuted(bool muted);
bool audioMuted();

#pragma once
#include <Arduino.h>

enum DareViewStatus {
  DARE_IDLE,
  DARE_LOADING,
  DARE_READY,
  DARE_ERROR,
};

struct DareViewState {
  DareViewStatus status = DARE_IDLE;
  String display = "Tap for one tiny dare.";
  String why = "context -> tap -> dare -> move";
  String color = "#8338EC";
  int timer_seconds = 0;
  String footer = "Touch screen or press KEY";
};

bool displayBegin();
void displayTick(const DareViewState& s);
void displaySleep(bool sleep);
void displayShowProvisioning(const char* ssid);
void displayShowBoot(const char* msg);

#pragma once
#include <pebble.h>
#include "persist_keys.h"

#define SETTINGS_VERSION 1

// Increment SETTINGS_VERSION when reordering or removing any members.
// Do not change SETTINGS_VERSION if just adding new members to the end
typedef struct ClaySettings {
  int version;
  int CentreSize;
  int InnerCentreSize;
  int HandThickness;
  int BackSize;
  int BackLen;
  int ComplicationFontSizeAdj;
  int MinuteHandUpdateIntervalSec;
  char PosLeft[4];
  char PosRight[4];
  char PosTop[4];
  char PosBottom[4];

  GColor SyncedAlarmPinColor;
  GColor CalendarPinColor;
  GColor BackgroundColor;
  GColor ComplicationBorderColor;
  GColor ComplicationShadowColor;
  GColor MinuteHandShadowColor;
  GColor MinorTickColor;
  GColor MajorTickColor;
  GColor DateColor;
  GColor HourDigitsColor;
  GColor MinuteDigitsColor;
  GColor HourHandColor;
  GColor MinutesHandColor;
  GColor BatteryLineColor;
  GColor BTQTColor;
  GColor WatchDialWindowColor;
  GColor MinimizedMajorTickColor;
  GColor ComplicationBackgroundColor;
  GColor LocalAlarmPinColor;

  char LogoText[18];
  bool EnableDate;
  bool EnableBattery;
  bool EnableBatteryLine;
  bool EnableLogo;
  bool VibeOn;
  bool AddZero12h;
  bool RemoveZero24h;
  bool showlocalAMPM;
  bool showMinorTick;
  bool showMajorTick;
  bool DigitalHour;
  bool SmoothMinuteHand;
  bool OrbitComplications;
  bool EnableAlarmCalendarSync;
  bool BWShadowOn;
  bool ShadowOn;
  bool ForegroundShape;
  bool TimelineAlarmPin;
  bool TimelineTimerPin;
  bool ShowWatchDialWindow;
  bool BlankFaceMode;
  bool QuietTimeBlankFace;
} ClaySettings;

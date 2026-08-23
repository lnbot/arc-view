module.exports = [
  {
    "type": "heading",
    "defaultValue": "monologue-orbital"
  },
  {
    "type": "text",
    "defaultValue": "<p>by lnbot</p>"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Features"
      },
      {
        "type": "toggle",
        "label": "Watchface Style",
        "messageKey": "ForegroundShape",
        "description": "Off = Rectangular, On = Round",
        "capabilities": [ "RECT" ],
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Digital Time Style",
        "messageKey": "DigitalHour",
        "description": "Off = Digital Minute, Hour Hand, On = Digital Hour, Minute Hand",
        "defaultValue": true
      },
      {
        "type": "slider",
        "label": "Minute hand updates per minute",
        "messageKey": "MinuteHandUpdatesPerMin",
        "description": "Smoother minute hand movement at the expense of battery life",
        "defaultValue": 6,
        "min": 1,
        "max": 15,
        "step": 1
      },
      {
        "type": "toggle",
        "label": "Orbiting Complications",
        "messageKey": "OrbitComplications",
        "description": "Complications orbit the center of the watch face",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Orbiting Watch Dial Window",
        "messageKey": "ShowWatchDialWindow",
        "description": "Window that follows the minute hand and reveals the dial ticks underneath. Only enabled with round watchface and orbiting complications.",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Alarm, Timer, and Calendar Sync",
        "messageKey": "EnableAlarmCalendarSync",
        "description": "Show upcoming alarms, timers, and calendar events on the watch face.  Requires companion app to be installed on the phone.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Alarm Timeline Pin",
        "messageKey": "TimelineAlarmPin",
        "description": "Place a 'Phone alarm' pin on the Timeline whenever a new alarm time is synced.  Expired and deleted alarms are removed from the timeline.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Timer Timeline Pin",
        "messageKey": "TimelineTimerPin",
        "description": "Place a 'Phone timer' pin on the Timeline whenever a new timer is synced.  Expired and deleted timers are removed from the timeline.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Add leading zero to 12h time",
        "messageKey": "AddZero12h",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Remove leading zero from 24h time",
        "messageKey": "RemoveZero24h",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Date visible",
        "messageKey": "EnableDate",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Logo visible",
        "messageKey": "EnableLogo",
        "defaultValue": true
      },
      {
         "type": "input",
         "messageKey": "LogoText",
         "defaultValue": "pebble",
         "label": "Custom Logo Text",
         "attributes": {
           "placeholder": "pebble"
         }
      },
      {
        "type": "toggle",
        "label": "Battery Value visible",
        "messageKey": "EnableBattery",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Battery Line visible",
        "messageKey": "EnableBatteryLine",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show Major Tickmarks",
        "messageKey": "showMajorTick",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show Minor Tickmarks",
        "messageKey": "showMinorTick",
        "defaultValue": true
      },
      {
        "type": "slider",
        "label": "Complication Font Size Adjustment",
        "messageKey": "ComplicationFontSizeAdj",
        "defaultValue": 0,
        "description": "Default = 0",
        "min": -5,
        "max": 10,
        "step": 1
      },
      {
        "type": "toggle",
        "messageKey": "VibeOn",
        "label": "Vibrate on Bluetooth disconnect during Quiet Time",
        "description": "If set to off, will still vibrate on disconnect, but not during quiet time",
        "defaultValue": false
      }
    ]
  },
   {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Layout"
      },
        {
        "type": "select",
        "messageKey": "PosTop",
        "label": "Top Position",
        "description": "Top position is ignored when Orbiting Complications is enabled",
        "defaultValue": "em",
            "options": [
          {
            "label": "Empty",
            "value": "em"
          },
          {
            "label": "Time Digits",
            "value": "hr"
          },
          {
            "label": "AM/PM marker",
            "value": "ap"
          },
          {
            "label": "Date",
            "value": "dt"
          },
          {
            "label": "Battery & Logo",
            "value": "lo"
          }
        ]
        },
        {
        "type": "select",
        "messageKey": "PosRight",
        "label": "Right Position",
        "defaultValue": "lo",
            "options": [
          {
            "label": "Empty",
            "value": "em"
          },
          {
            "label": "Time Digits",
            "value": "hr"
          },
          {
            "label": "AM/PM marker",
            "value": "ap"
          },
          {
            "label": "Date",
            "value": "dt"
          },
          {
            "label": "Battery & Logo",
            "value": "lo"
          }
        ]
        },
        {
        "type": "select",
        "messageKey": "PosBottom",
        "label": "Bottom Position",
        "defaultValue": "dt",
            "options": [
          {
            "label": "Empty",
            "value": "em"
          },
          {
            "label": "Time Digits",
            "value": "hr"
          },
          {
            "label": "AM/PM marker",
            "value": "ap"
          },
          {
            "label": "Date",
            "value": "dt"
          },
          {
            "label": "Battery & Logo",
            "value": "lo"
          }
        ]
        },
        {
        "type": "select",
        "messageKey": "PosLeft",
        "label": "Left Position",
        "defaultValue": "hr",
            "options": [
          {
            "label": "Empty",
            "value": "em"
          },
          {
            "label": "Time Digits",
            "value": "hr"
          },
          {
            "label": "AM/PM marker",
            "value": "ap"
          },
          {
            "label": "Date",
            "value": "dt"
          },
          {
            "label": "Battery & Logo",
            "value": "lo"
          }
        ]
        },
        {
        "type": "toggle",
        "messageKey": "BlankFaceMode",
        "label": "Blank Face Mode",
        "description": "Blanks out complications, forces hand to be hour hand when active.",
        "defaultValue": false
        },
        {
        "type": "toggle",
        "messageKey": "QuietTimeBlankFace",
        "label": "Blank Face during Quiet Time",
        "description": "Blanks out complications only during Quiet Time.  Forced hour hand only during Quiet Time.",
        "defaultValue": false
        }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  },
  {
    "type": "section",
    "capabilities": [ "COLOR" ],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colours"
      },
      {
        "type": "toggle",
        "label": "Minute Hand Shadow On",
        "messageKey": "ShadowOn",
        "defaultValue": true
      },
      {
        "type": "radiogroup",
        "id": "ThemeSelect",
        "defaultValue": "wh",
        "label": "COLOUR THEME SELECT",
        "description": "Changes to *built-in themes will be applied, but not saved to the theme.",
        "options": [
          {
            "label": "%TemplateLabel",
            "value": "%TemplateValue"
          }
        ],
        "attributes": {
          "id": "ThemeSelectRadioGroup",
        }
      },
      {
         "type": "input",
         "id": "ThemeCopyDelete",
         "label": "New theme name for 'Copy theme' or type 'delete' before 'Delete theme'",
         "attributes": {
           "placeholder": "New theme name / 'delete'"
         }
      },
      {
        "type": "button",
        "id": "CopyThemeButton",
        "primary": false,
        "defaultValue": "Copy theme"
      },
      {
        "type": "button",
        "id": "DeleteThemeButton",
        "primary": false,
        "defaultValue": "Delete theme"
      },
      {
        "type": "color",
        "label": "Background Colour",
        "messageKey": "BackgroundColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Complication Border Color",
        "messageKey": "ComplicationBorderColor",
        "description": "Set to the background color to disable border",
        "defaultValue": "AAAAAA"
      },
      {
        "type": "color",
        "label": "Complication Background Color",
        "messageKey": "ComplicationBackgroundColor",
        "description": "Set to the background color to disable",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Complication Shadow Color",
        "messageKey": "ComplicationShadowColor",
        "description": "Enabled with minute hand shadow switch",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Watch Dial Window Color",
        "messageKey": "WatchDialWindowColor",
        "description": "Background color for the watch dial window",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Minute Hand Shadow Colour",
        "messageKey": "MinuteHandShadowColor",
        "defaultValue": "AAAAAA"
      },
      {
        "type": "color",
        "label": "Major Tickmark Colour",
        "messageKey": "MajorTickColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minor Tickmark Colour",
        "messageKey": "MinorTickColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minimized Major Tickmark Colour (outside of Watch Dial Window)",
        "messageKey": "MinimizedMajorTickColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Hours Digits Colour",
        "messageKey": "HourDigitsColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Minute Hand Colour",
        "messageKey": "MinutesHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Date, Battery & Logo Text Colour",
        "messageKey": "DateColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Battery Line Colour",
        "messageKey": "BatteryLineColor",
        "defaultValue": "FF5500"
      },
      {
        "type": "color",
        "label": "Quiet Time and Bluetooth Icon Colour",
        "messageKey": "BTQTColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Watch Alarm Pin Colour",
        "messageKey": "LocalAlarmPinColor",
        "defaultValue": "550055",
      },
      {
        "type": "color",
        "label": "Phone Alarm and Timer Pin Colour",
        "messageKey": "SyncedAlarmPinColor",
        "defaultValue": "FF8800",
      },
      {
        "type": "color",
        "label": "Calendar Pin Colour",
        "messageKey": "CalendarPinColor",
        "defaultValue": "0055FF",
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Analogue Hand Style"
      },
      {
        "type": "slider",
        "label": "Minute Hand Thickness",
        "messageKey": "HandThickness",
        "description": "Default = 2",
        "defaultValue": 2,
        "min": 1,
        "max": 5,
        "step": 1
      },
      {
        "type": "slider",
        "label": "Hand Centre Radius",
        "messageKey": "CentreSize",
        "defaultValue": 0,
        "description": "Default = 0",
        "min": 0,
        "max": 15,
        "step": 1
      },
      {
        "type": "slider",
        "label": "Hand Centre Inner Radius",
        "messageKey": "InnerCentreSize",
        "defaultValue": 0,
        "description": "Default = 0",
        "min": 0,
        "max": 9,
        "step": 1
      },
      {
        "type": "slider",
        "label": "Hand End Radius",
        "messageKey": "BackSize",
        "defaultValue": 0,
        "description": "Default = 0",
        "min": 0,
        "max": 9,
        "step": 1
      },
      {
        "type": "slider",
        "label": "Hand End Length",
        "messageKey": "BackLen",
        "defaultValue": 4,
        "description": "Default = 4",
        "min": -60,
        "max": 40,
        "step": 2
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  },
  {
    "type": "input",
    "id": "XCLAYUserThemes",
    "messageKey": "XCLAYUserThemes",
    "attributes": {
      "id": "UserThemesInput"
    }
  },
  {
    "type": "input",
    "id": "XCLAYActiveTheme",
    "messageKey": "XCLAYActiveTheme",
    "attributes": {
      "id": "ActiveThemeInput"
    }
  },
];

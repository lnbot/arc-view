module.exports = [
  {
    "type": "heading",
    "defaultValue": "monologue-orbital"
  },
  {
    "type": "text",
    "defaultValue": "<p>by astosia</p>"
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
        "capabilities": [
          "RECT"
        ],
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
        "defaultValue": 1,
        "min": 1,
        "max": 15,
        "step": 1
      },
      {
        "type": "toggle",
        "label": "Orbiting Complications",
        "messageKey": "OrbitComplications",
        "description": "Complications orbit the center of the watch face",
        "capabilities": [ "NOT_BW" ],
        "defaultValue": false
      },
      {
        "type": "toggle",
        "label": "Alarm, Timer, and Calendar Sync",
        "messageKey": "EnableAlarmCalendarSync",
        "description": "Show upcoming alarms, timers, and calendar events on the watch face.  Requires companion app to be installed on the phone.",
        "capabilities": [ "NOT_BW" ],
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
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
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
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
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
        "messageKey": "PosRight",
        "label": "Right Position",
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
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
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
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
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
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
        "type": "select",
        "messageKey": "PosTop",
        "label": "Top Position",
        "description": "Top position is ignored when Orbiting Complications is enabled",
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
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
        "messageKey": "PosRight",
        "label": "Right Position",
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
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
        "messageKey": "PosBottom",
        "label": "Bottom Position",
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
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
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
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
        "messageKey": "ThemeSelect",
        "defaultValue": "wh",
        "label": "COLOUR THEME SELECT",
        "options": [
          {
            "label": "White Background",
            "value": "wh"
          },
          {
            "label": "Black Background",
            "value": "bl"
          },
          {
              "label": "Blue Background",
              "value": "bu"
          },
          {
              "label": "Purple Background",
              "value": "pl"
          },
          {
              "label": "Black & Green",
              "value": "gr"
          },
          {
            "label": "Custom Colours",
            "value": "cu"
          }
        ]
      },
      {
           "type": "heading",
           "defaultValue": "Custom Colours"
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
        "label": "Complication Shadow Color",
        "messageKey": "ComplicationShadowColor",
        "description": "Enabled with minute hand shadow switch",
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
        "label": "Alarm and Timer Pin Colour",
        "messageKey": "AlarmPinColor",
        "defaultValue": "FF8800",
        "allowGray": false,
        "sunlight": false
      },
      {
        "type": "color",
        "label": "Calendar Pin Colour",
        "messageKey": "CalendarPinColor",
        "defaultValue": "0055FF",
        "allowGray": false,
        "sunlight": false
      }
    ]
  },
  {
    "type": "section",
    "capabilities": [ "BW" ],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colours"
      },
      {
        "type": "toggle",
       "label": "Minute Hand Shadow On",
       "messageKey": "BWShadowOn",
       "defaultValue": true
      },
       {
         "type": "radiogroup",
         "messageKey": "BWThemeSelect",
         "defaultValue": "wh",
         "label": "COLOUR THEME SELECT",
         "options": [
           {
             "label": "White Background",
             "value": "wh"
           },
           {
             "label": "Black Background",
             "value": "bl"
           },
           {
             "label": "Custom Colours",
             "value": "cu"
           }
         ]
       },
          {
            "type": "heading",
            "defaultValue": "Custom Colours"
          },
      {
        "type": "color",
        "label": "Background Colour",
        "messageKey": "BWBackgroundColor1",
        "defaultValue": "FFFFFF",
        "allowGray": true
      },
      {
        "type": "color",
        "label": "Minute Hand Shadow Colour",
        "messageKey": "BWMinuteHandShadowColor",
        "defaultValue": "AAAAAA",
        "allowGray": true
      },
      {
        "type": "color",
        "label": "Tickmark Colour",
        "messageKey": "BWMajorTickColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minute Hand & Battery Line Colour",
        "messageKey": "BWMinHandBatLineColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Hours Digits Colour",
        "messageKey": "BWHourDigitsColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Date, Battery Value & Logo Text Colour",
        "messageKey": "BWDateColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Quiet Time and Bluetooth Icon Colour",
        "messageKey": "BWBTQTColor",
        "defaultValue": "000000"
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
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
        "label": "Hand Centre Radius",
        "messageKey": "CentreSize",
        "defaultValue": 7,
        "description": "Default = 7",
        "min": 1,
        "max": 11,
        "step": 1
      },
      {
        "type": "slider",
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
        "label": "Hand Centre Radius",
        "messageKey": "CentreSize",
        "defaultValue": 9,
        "description": "Default = 9",
        "min": 1,
        "max": 15,
        "step": 1
      },
      {
        "type": "slider",
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
        "label": "Hand Centre Inner Radius",
        "messageKey": "InnerCentreSize",
        "defaultValue": 1,
        "description": "Default = 1",
        "min": 0,
        "max": 9,
        "step": 1
      },
      {
        "type": "slider",
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
        "label": "Hand Centre Inner Radius",
        "messageKey": "InnerCentreSize",
        "defaultValue": 2,
        "description": "Default = 2",
        "min": 0,
        "max": 9,
        "step": 1
      },
      {
        "type": "slider",
        "label": "Hand End Radius",
        "messageKey": "BackSize",
        "defaultValue": 4,
        "description": "Default = 4",
        "min": 0,
        "max": 9,
        "step": 1
      },
      {
        "type": "slider",
        "label": "Hand End Length",
        "messageKey": "BackLen",
        "capabilities": ["NOT_PLATFORM_APLITE", "NOT_PLATFORM_BASALT", "NOT_PLATFORM_CHALK", "NOT_PLATFORM_DIORITE", "NOT_PLATFORM_FLINT"],
        "defaultValue": 28,
        "description": "Default = 28",
        "min": 0,
        "max": 40,
        "step": 2
      },
      {
        "type": "slider",
        "label": "Hand End Length",
        "messageKey": "BackLen",
        "capabilities": ["NOT_PLATFORM_GABBRO", "NOT_PLATFORM_EMERY"],
        "defaultValue": 22,
        "description": "Default = 22",
        "min": 0,
        "max": 30,
        "step": 2
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  },
];

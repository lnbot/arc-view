const BuiltinThemes = {
  version: 1,
  themes: {
    blue: {
      name: "Blue-gray background",
      BackgroundColor: "Liberty",
      ComplicationBorderColor: "LightGray",
      ComplicationBackgroundColor: "White",
      ComplicationShadowColor: "DarkGray",
      WatchDialWindowColor: "White",
      DateColor: "Black",
      HourDigitsColor: "OxfordBlue",
      MinuteDigitsColor: "BulgarianRose",
      HourHandColor: "OxfordBlue",
      MinutesHandColor: "BulgarianRose",
      MinuteHandShadowColor: "LightGray",
      MajorTickColor: "Black",
      MinimizedMajorTickColor: "White",
      MinorTickColor: "Black",
      BatteryLineColor: "IslamicGreen",
      BTQTColor: "Black",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "BrilliantRose",
      CalendarPinColor: "ChromeYellow",
    },
    white: {
      name: "White Background",
      BackgroundColor: "White",
      ComplicationBorderColor: "LightGray",
      ComplicationBackgroundColor: "White",
      ComplicationShadowColor: "DarkGray",
      DateColor: "DarkGray",
      MinuteDigitsColor: "CobaltBlue",
      HourDigitsColor: "CobaltBlue",
      MinutesHandColor: "CobaltBlue",
      HourHandColor: "CobaltBlue",
      MinuteHandShadowColor: "BabyBlueEyes",
      MajorTickColor: "CobaltBlue",
      MinimizedMajorTickColor: "CobaltBlue",
      MinorTickColor: "CobaltBlue",
      BatteryLineColor: "Orange",
      BTQTColor: "DarkGray",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "BulgarianRose",
      CalendarPinColor: "WindsorTan",
      WatchDialWindowColor: "ElectricBlue",
    },
    barewhite: {
      name: "Bare White",
      BackgroundColor: "White",
      ComplicationBorderColor: "LightGray",
      ComplicationBackgroundColor: "White",
      ComplicationShadowColor: "DarkGray",
      DateColor: "Black",
      MinuteDigitsColor: "BulgarianRose",
      HourDigitsColor: "OxfordBlue",
      MinutesHandColor: "BulgarianRose",
      HourHandColor: "OxfordBlue",
      MinuteHandShadowColor: "LightGray",
      MajorTickColor: "Black",
      MinimizedMajorTickColor: "White",
      MinorTickColor: "Black",
      BatteryLineColor: "IslamicGreen",
      BTQTColor: "Black",
      LocalAlarmPinColor: "DukeBlue",
      SyncedAlarmPinColor: "DarkCandyAppleRed",
      CalendarPinColor: "MidnightGreen",
      WatchDialWindowColor: "White",
    },
    green: {
      name: "Green background",
      BackgroundColor: "KellyGreen",
      ComplicationBorderColor: "LightGray",
      ComplicationBackgroundColor: "White",
      ComplicationShadowColor: "DarkGray",
      WatchDialWindowColor: "White",
      DateColor: "Black",
      HourDigitsColor: "OxfordBlue",
      MinuteDigitsColor: "BulgarianRose",
      HourHandColor: "OxfordBlue",
      MinutesHandColor: "BulgarianRose",
      MinuteHandShadowColor: "LightGray",
      MajorTickColor: "Black",
      MinimizedMajorTickColor: "White",
      MinorTickColor: "Black",
      BatteryLineColor: "IslamicGreen",
      BTQTColor: "Black",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "BrilliantRose",
      CalendarPinColor: "ChromeYellow",
    },
  }
};

var Colors = require('./colors.js');
const StorageKey = 'AppColorThemes';

function updateThemesInStorage() {
  var task = 'start';

  try {
    var storedThemesJson = localStorage.getItem(StorageKey);
    var storedThemes = null;

    try {
      var obj = JSON.parse(storedThemesJson);
      if ('version' in obj) {
        storedThemes = obj;
      } else {
        console.log(`updateThemesInStorage: invalid stored theme ${obj.toString()}`);
      }
    } catch (e) {}


    if (storedThemes && storedThemes.version === BuiltinThemes.version) {
      console.log(`updateThemesInStorage: Stored theme version=${storedThemes.version}`);
      Object.assign(storedThemes, { cached: 1, error: 'OK', meta: '' });
      return storedThemes;
    }

    var processedThemes = { version: BuiltinThemes.version, themes: {} };

    for (var tkey of Object.keys(BuiltinThemes.themes)) {
      var pt = {};
      var currTheme = BuiltinThemes.themes[tkey];

      task = `loop: tkey=${tkey}`;
      // Resolve all colors, but otherwise, a straight copy
      for (var k of Object.keys(currTheme)) {
        if (!k.endsWith('Color')) {
          pt[k] = currTheme[k];
          continue;
        }
        pt[k] = Colors.resolve(currTheme[k], '#FFFFFF');
      }
      processedThemes.themes[tkey] = pt;
      task = 'themeadded';
    }

    console.log(`updateThemesInStorage: Processed themes, version=${processedThemes.version}`);
    task = 'storage';
    localStorage.setItem(StorageKey, JSON.stringify(processedThemes));
    Object.assign(processedThemes, { cached: 0, error: 'OK', meta: '' });
    return processedThemes;

  } catch (e) {
    return { version: 0, themes: [], error: e.toString(), meta: task, cached: 0 };
  }
}

module.exports = {
  fetchBuiltinThemes: updateThemesInStorage,
}

const BuiltinThemes = {
  version: 1,
  themes: {
    wh: {
      name: "White Background",
      BackgroundColor1: "White",
      ComplicationBorderColor: "LightGray",
      ComplicationBackgroundColor: "White",
      ComplicationShadowColor: "DarkGray",
      DateColor: "DarkGray",
      HourDigitsColor: "CobaltBlue",
      MinutesHandColor: "CobaltBlue",
      MinuteHandShadowColor: "BabyBlueEyes",
      MajorTickColor: "CobaltBlue",
      MinimizedMajorTickColor: "CobaltBlue",
      MinorTickColor: "CobaltBlue",
      BatteryLineColor: "Orange",
      BTQTColor: "DarkGray",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "RichBrilliantLavender",
      CalendarPinColor: "CadetBlue",
      WatchDialWindowColor: "VeryLightBlue",
    },
    bl: {
      name: "Black Background",
      BackgroundColor1: "Black",
      ComplicationBorderColor: "DarkGray",
      ComplicationBackgroundColor: "Black",
      ComplicationShadowColor: "LightGray",
      DateColor: "WindsorTan",
      HourDigitsColor: "Yellow",
      MinutesHandColor: "Yellow",
      MinuteHandShadowColor: "Black",
      MajorTickColor: "Yellow",
      MinimizedMajorTickColor: "Yellow",
      MinorTickColor: "DarkGray",
      BatteryLineColor: "Yellow",
      BTQTColor: "LightGray",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "RichBrilliantLavender",
      CalendarPinColor: "CadetBlue",
      WatchDialWindowColor: "DarkGray",
    },
    bu: {
      name: "Blue Background",
      BackgroundColor1: "DukeBlue",
      ComplicationBorderColor: "DukeBlue",
      ComplicationBackgroundColor: "DukeBlue",
      ComplicationShadowColor: "DukeBlue",
      DateColor: "White",
      HourDigitsColor: "Yellow",
      MinutesHandColor: "Yellow",
      MinuteHandShadowColor: "OxfordBlue",
      MajorTickColor: "Yellow",
      MinimizedMajorTickColor: "Yellow",
      MinorTickColor: "PictonBlue",
      BatteryLineColor: "Red",
      BTQTColor: "PictonBlue",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "RichBrilliantLavender",
      CalendarPinColor: "CadetBlue",
      WatchDialWindowColor: "VeryLightBlue",
    },
    pl: {
      name: "Purple Background",
      BackgroundColor1: "Purple",
      ComplicationBorderColor: "Purple",
      ComplicationBackgroundColor: "Purple",
      ComplicationShadowColor: "Purple",
      DateColor: "RichBrilliantLavender",
      HourDigitsColor: "RichBrilliantLavender",
      MinutesHandColor: "RichBrilliantLavender",
      MinuteHandShadowColor: "ImperialPurple",
      MajorTickColor: "RichBrilliantLavender",
      MinorTickColor: "ImperialPurple",
      MinimizedMajorTickColor: "ImperialPurple",
      BatteryLineColor: "BulgarianRose",
      BTQTColor: "ImperialPurple",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "RichBrilliantLavender",
      CalendarPinColor: "CadetBlue",
      WatchDialWindowColor: "PastelYellow",
    },
    gr: {
      name: "Black and Green",
      BackgroundColor1: "Black",
      ComplicationBorderColor: "ArmyGreen",
      ComplicationBackgroundColor: "Black",
      ComplicationShadowColor: "MidnightGreen",
      DateColor: "Green",
      HourDigitsColor: "BrightGreen",
      MinutesHandColor: "BrightGreen",
      MinuteHandShadowColor: "DarkGreen",
      MajorTickColor: "BrightGreen",
      MinorTickColor: "DarkGreen",
      MinimizedMajorTickColor: "BrightGreen",
      BatteryLineColor: "PastelYellow",
      BTQTColor: "DarkGreen",
      LocalAlarmPinColor: "ScreaminGreen",
      SyncedAlarmPinColor: "RichBrilliantLavender",
      CalendarPinColor: "CadetBlue",
      WatchDialWindowColor: "MayGreen",
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

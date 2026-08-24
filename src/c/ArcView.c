#include <pebble.h>
#include "ArcView.h"
#include "alarm_calendar_sync.h"
#include "utils/storage_utils.h"
#include "utils/weekday.h"
#include "utils/MathUtils.h"
#include "message_keys.auto.h"
#include "src/resource_ids.auto.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>

// Turns out using ticks and quick returning on 90% of callbacks is still
// more battery efficient than using app timer
//#define SUB_MINUTE_USE_APPTIMER
#define SUB_MINUTE_USE_TICK

// Main window and layers
static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static Layer *s_date_battery_logo_layer;
static Layer *s_alarm_cal_pin_layer;
static GRect bounds;
// Fonts

// Smooth interval for updating the minute hand
const int SMOOTH_WAKEUP_COOKIE = 1;

static FFont* Date_Font;
static FFont* FontBTQTIconsFctx;

// Time and date variables
static struct tm prv_tm;
static struct tm *prv_tick_time = &prv_tm;
static int current_date;
static int s_weekday;
static int seconds;
static int minutes;
static int hours;   //12h modulo
static int s_hours; //24h version
static int hand_angle_native;

static ClaySettings settings;

#ifdef SUB_MINUTE_USE_APPTIMER
static AppTimer *sub_minute_timer = NULL;
#elif defined(SUB_MINUTE_USE_TICK)
static int sub_minute_interval;
#endif

// Date position struct for different platforms
typedef struct {
  int font_size_digits;
  int font_size_battery;
  int font_size_date;
  int font_size_logo;
  int font_size_btqt;
  int battery_line;
  int analogue_hand_a;
  int analogue_hand_b;
  int analogue_hand_c;
  int hands_shadow;
  int corner_radius_secondshand;
  int corner_radius_majortickrect;
  int corner_radius_minortickrect;
  int majortickrect_w;
  int majortickrect_h;
  int minortickrect_w;
  int minortickrect_h;
  int tick_inset_outer;
  int HandCentreOuterRadius;
  int HandCentreInnerRadius;
  int ComplicationBorderAdj;
  int ComplicationDistanceAdj;
  int ComplicationOrbitSizeAdj;
  int ComplicationDialWindowSizeAdj;
  int ComplicationDialWindowDistanceAdj;
} UIConfig;

#ifdef PBL_PLATFORM_EMERY
static const UIConfig config = {
.font_size_digits = 36,
.font_size_battery = 14,
.font_size_date = 12,
.font_size_logo = 10,
.font_size_btqt = 14,
.battery_line = 63, //sized to the width of the default logo TITANIUM
.analogue_hand_a = 1,  //was 20
.analogue_hand_b = 4,
.analogue_hand_c = 20,
.hands_shadow = 2,
.corner_radius_secondshand = 20,
.corner_radius_majortickrect = 20,
.corner_radius_minortickrect = 20,
.majortickrect_w = 86,
.majortickrect_h = 100,
.minortickrect_w = 90,
.minortickrect_h = 104,
.tick_inset_outer = -10,
.HandCentreOuterRadius = 0,
.HandCentreInnerRadius = 0,
.ComplicationBorderAdj = 4,
.ComplicationDistanceAdj = 6,
.ComplicationOrbitSizeAdj = 2,
.ComplicationDialWindowSizeAdj = 2,
.ComplicationDialWindowDistanceAdj = 2,
};
#else //defined(PBL_PLATFORM_GABBRO)
static const UIConfig config = {
.font_size_digits = 44,
.font_size_battery = 17,
.font_size_date = 15,
.font_size_logo = 13,
.font_size_btqt = 18,
.battery_line = 63,
.analogue_hand_a = 3+8,
.analogue_hand_b = 4,
.hands_shadow = 2,
.analogue_hand_c = 40,
.HandCentreOuterRadius = 0,
.HandCentreInnerRadius = 0,
.ComplicationBorderAdj = 2,
.ComplicationDistanceAdj = 2,
.ComplicationOrbitSizeAdj = 3,
.ComplicationDialWindowSizeAdj = 1,
.ComplicationDialWindowDistanceAdj = 4,
};
#endif

bool connected = true;
time_t next_alarm_time = 0;

//function prototypes

static void prv_save_settings(void);
static void prv_default_settings(void);
static void prv_load_settings(void);
static bool prv_restore_from_settings_dict(void);
static void prv_parse_settings_dict(DictionaryIterator* iter);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
#if defined(SUB_MINUTE_USE_APPTIMER)
static void apptimer_handler(void *data);
#endif
static void bg_update_proc(Layer *layer, GContext *ctx);
static void update_logo_date_battery_fctx_layer(Layer *layer, GContext * ctx);
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_alarm_cal_pins(Layer *layer, GContext *ctx);
static int calculate_hand_angle(struct tm *tick_time);
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color);
static void draw_hand_center(GContext *ctx, GColor outer_color, GColor inner_color);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);
static bool use_minute_hand();
static bool skip_render_complications();

// Save settings to persistent storage
static void prv_save_settings(void) {
  //int size = 0 +
  persist_write_data_multi(SETTINGS_KEY, &settings, sizeof(settings), SETTINGS_MAX_BLOCKS);
  //APP_LOG(APP_LOG_LEVEL_INFO, "SaveSettings: wrote %d bytes", size);
}


// Set default settings
static void prv_default_settings(void) {
  settings.version = SETTINGS_VERSION;
  settings.EnableDate = true;
  settings.EnableBattery = true;
  settings.EnableBatteryLine = true;
  settings.EnableLogo = true;
  snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "pebble");
  settings.BackgroundColor = GColorLiberty;
  settings.ComplicationBorderColor = GColorLightGray;
  settings.ComplicationBackgroundColor = GColorWhite;
  settings.ComplicationShadowColor = GColorDarkGray;
  settings.MinuteHandShadowColor = GColorLightGray;
  settings.MinorTickColor = GColorBlack;
  settings.DateColor = GColorBlack;
  settings.HourDigitsColor = GColorOxfordBlue;
  settings.MinuteDigitsColor = GColorBulgarianRose;
  settings.HourHandColor = GColorOxfordBlue;
  settings.MinutesHandColor = GColorBulgarianRose;
  settings.MajorTickColor = GColorBlack;
  settings.MinimizedMajorTickColor = GColorWhite;
  settings.BatteryLineColor = GColorIslamicGreen;
  settings.BTQTColor = GColorBlack;
  settings.showMajorTick = true;
  settings.showMinorTick = true;
  snprintf(settings.PosTop, sizeof(settings.PosTop), "%s", "ap");
  snprintf(settings.PosLeft, sizeof(settings.PosLeft), "%s", "hr");
  snprintf(settings.PosRight, sizeof(settings.PosRight), "%s", "lo");
  snprintf(settings.PosBottom, sizeof(settings.PosBottom), "%s", "dt");
  settings.ShadowOn = true;
  settings.VibeOn = false;
  settings.AddZero12h = false;
  settings.RemoveZero24h = false;
  settings.ForegroundShape = true;  //true = round, false = rect
  settings.CentreSize = config.HandCentreOuterRadius;
  settings.InnerCentreSize = config.HandCentreInnerRadius;
  settings.HandThickness = 2;
  settings.DigitalHour = true;
  settings.BackSize = 0;
  settings.BackLen = config.analogue_hand_b;
  settings.ComplicationFontSizeAdj = 0;
  settings.SmoothMinuteHand = true;
  settings.MinuteHandUpdateIntervalSec = 10;
  settings.OrbitComplications = true;
  settings.EnableAlarmCalendarSync = false;
  settings.LocalAlarmPinColor = GColorScreaminGreen;
  settings.SyncedAlarmPinColor = GColorBrilliantRose;
  settings.CalendarPinColor = GColorChromeYellow;
  settings.TimelineAlarmPin = false;
  settings.TimelineTimerPin = false;
  settings.ShowWatchDialWindow = true;
  settings.WatchDialWindowColor = GColorWhite;
  settings.BlankFaceMode = false;
  settings.QuietTimeBlankFace = false;
}

static bool use_minute_hand() {
  return !skip_render_complications() && settings.DigitalHour;
}

static bool skip_render_complications() {
  return (settings.BlankFaceMode && !settings.QuietTimeBlankFace) ||
    (settings.BlankFaceMode && settings.QuietTimeBlankFace && quiet_time_is_active());
}

static void bluetooth_vibe_icon (bool connected) {
  layer_mark_dirty(s_date_battery_logo_layer);

  if((!connected && !quiet_time_is_active()) ||(!connected && quiet_time_is_active() && settings.VibeOn)) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

// Load settings from persistent storage
static void prv_load_settings(void) {
  int version = -1;

  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &version, sizeof(version));
  APP_LOG(APP_LOG_LEVEL_INFO, "LoadSettings: version=%d", version);

  // Nothing there, so just bail.
  if (version == -1) {
    prv_save_settings();
    return;
  }

  if (version == settings.version) {
    persist_read_data_multi(SETTINGS_KEY, &settings, sizeof(settings), SETTINGS_MAX_BLOCKS);
    return;
  }

  // Fallback: incompatible native settings, but reparse the last received settings dict
  bool restored = prv_restore_from_settings_dict();
  APP_LOG(APP_LOG_LEVEL_INFO, "LoadSettings: version mismatch, restore from dict (success=%d)", restored);
  prv_save_settings();
}

static bool prv_set_color(Tuple *val, GColor *target) {
  if (val) {
    *target = GColorFromHEX(val->value->int32);
    return true;
  }
  return false;
}

static bool prv_set_bool(Tuple *val, bool *target) {
  if (val) {
    *target = val->value->int32 != 0;
    return true;
  }
  return false;
}

static bool prv_set_int32(Tuple *val, int *target) {
  if (val) {
    *target = (int) val->value->int32;
    return true;
  }
  return false;
}

// AppMessage inbox handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  prv_parse_settings_dict(iter);

  size_t size = (char *)iter->end - (char *)iter->dictionary;
  persist_write_data_multi(SETTINGS_DICT_KEY, iter->dictionary, size, SETTINGS_DICT_MAX_BLOCKS);
  //APP_LOG(APP_LOG_LEVEL_INFO, "Write settings dict size=%u", size);
}

static bool prv_restore_from_settings_dict() {
  PersistDictionary *pd = NULL;
  bool ret = false;
  size_t size;

  if (persist_read_data_multi(SETTINGS_DICT_KEY, &size, sizeof(size), SETTINGS_DICT_MAX_BLOCKS) != sizeof(size)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Settings dict not in persistent storage");
    goto cleanup;
  }

  pd = (PersistDictionary *)(size);
  if (pd == NULL) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Settings dict malloc() failed");
    goto cleanup;
  }

  DictionaryIterator iter;
  dict_read_begin_from_buffer(&iter, pd->dictionary, pd->size - sizeof(size));
  prv_parse_settings_dict(&iter);

cleanup:
  if (pd == NULL)
    free(pd);
  return ret;
}

static void prv_parse_settings_dict(DictionaryIterator *iter) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "ParseSettingsDict: Dict size=%u", (char *)iter->end - (char *)iter->dictionary);
  bool settings_changed = false;

  Tuple *vibe_t = dict_find(iter, MESSAGE_KEY_VibeOn);
  Tuple *enable_date_t = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_battery_t = dict_find(iter, MESSAGE_KEY_EnableBattery);
  Tuple *enable_battery_line_t = dict_find(iter, MESSAGE_KEY_EnableBatteryLine);
  Tuple *enable_logo_t = dict_find(iter, MESSAGE_KEY_EnableLogo);
  Tuple *logotext_t = dict_find(iter, MESSAGE_KEY_LogoText);
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  Tuple *comp_border_color_t = dict_find(iter, MESSAGE_KEY_ComplicationBorderColor);
  Tuple *comp_background_color_t = dict_find(iter, MESSAGE_KEY_ComplicationBackgroundColor);
  Tuple *comp_shadow_color_t = dict_find(iter, MESSAGE_KEY_ComplicationShadowColor);
  Tuple *bg_color2_t = dict_find(iter, MESSAGE_KEY_MinuteHandShadowColor);
  Tuple *text_color2_t = dict_find(iter, MESSAGE_KEY_MinorTickColor);
  Tuple *date_color_t = dict_find(iter, MESSAGE_KEY_DateColor);
  Tuple *hours_digits_color_t = dict_find(iter, MESSAGE_KEY_HourDigitsColor);
  Tuple *minute_digits_color_t = dict_find(iter, MESSAGE_KEY_MinuteDigitsColor);
  Tuple *minutes_hand_color_t = dict_find(iter, MESSAGE_KEY_MinutesHandColor);
  Tuple *hour_hand_color_t = dict_find(iter, MESSAGE_KEY_HourHandColor);
  Tuple *tick_color_t = dict_find(iter, MESSAGE_KEY_MajorTickColor);
  Tuple *battery_line_color_t = dict_find(iter, MESSAGE_KEY_BatteryLineColor);
  Tuple *btqt_color_t = dict_find(iter, MESSAGE_KEY_BTQTColor);
  Tuple *shadowon_t = dict_find(iter, MESSAGE_KEY_ShadowOn);
  Tuple *addzero12_t = dict_find(iter, MESSAGE_KEY_AddZero12h);
  Tuple *remzero24_t = dict_find(iter, MESSAGE_KEY_RemoveZero24h);
  Tuple *posleft_t = dict_find(iter, MESSAGE_KEY_PosLeft);
  Tuple *posright_t = dict_find(iter, MESSAGE_KEY_PosRight);
  Tuple *postop_t = dict_find(iter, MESSAGE_KEY_PosTop);
  Tuple *posbottom_t = dict_find(iter, MESSAGE_KEY_PosBottom);
  Tuple *majort_t = dict_find(iter, MESSAGE_KEY_showMajorTick);
  Tuple *minort_t = dict_find(iter, MESSAGE_KEY_showMinorTick);
  Tuple *fg_shape_t = dict_find(iter, MESSAGE_KEY_ForegroundShape);
  Tuple *dig_t = dict_find(iter,MESSAGE_KEY_DigitalHour);
  Tuple *hand_t = dict_find(iter, MESSAGE_KEY_HandThickness);
  Tuple *ocent_t = dict_find(iter, MESSAGE_KEY_CentreSize);
  Tuple *icent_t = dict_find(iter, MESSAGE_KEY_InnerCentreSize);
  Tuple *back_t = dict_find(iter, MESSAGE_KEY_BackSize);
  Tuple *backlen_t = dict_find(iter, MESSAGE_KEY_BackLen);
  Tuple *complication_font_size_adj_t = dict_find(iter, MESSAGE_KEY_ComplicationFontSizeAdj);
  Tuple *minute_hand_updates_per_min_t = dict_find(iter, MESSAGE_KEY_MinuteHandUpdatesPerMin);
  Tuple *orbit_complications_t = dict_find(iter, MESSAGE_KEY_OrbitComplications);
  Tuple *enable_alarm_calendar_sync_t = dict_find(iter, MESSAGE_KEY_EnableAlarmCalendarSync);
  Tuple *local_alarm_pin_color_t = dict_find(iter, MESSAGE_KEY_LocalAlarmPinColor);
  Tuple *synced_alarm_pin_color_t = dict_find(iter, MESSAGE_KEY_SyncedAlarmPinColor);
  Tuple *calendar_pin_color_t = dict_find(iter, MESSAGE_KEY_CalendarPinColor);
  Tuple *timeline_alarm_pin_t = dict_find(iter, MESSAGE_KEY_TimelineAlarmPin);
  Tuple *timeline_timer_pin_t = dict_find(iter, MESSAGE_KEY_TimelineTimerPin);
  Tuple *show_watch_dial_window_t = dict_find(iter, MESSAGE_KEY_ShowWatchDialWindow);
  Tuple *watch_dial_window_color_t = dict_find(iter, MESSAGE_KEY_WatchDialWindowColor);
  Tuple *minimized_major_tick_color_t = dict_find(iter, MESSAGE_KEY_MinimizedMajorTickColor);
  Tuple *blank_face_t = dict_find(iter,MESSAGE_KEY_BlankFaceMode);
  Tuple *qt_blank_face_t = dict_find(iter,MESSAGE_KEY_QuietTimeBlankFace);

  bool oldUseMinuteHand = use_minute_hand();

  if (dict_find(iter, MESSAGE_KEY_XCLAYUserThemes)) {
    // This is a potentially massive blob, and it should be filtered out
    APP_LOG(APP_LOG_LEVEL_WARNING, "XCLAY message found.  This should not be sent to the watchface.");
  }

  settings_changed |= prv_set_bool(fg_shape_t, &settings.ForegroundShape);
  settings_changed |= prv_set_bool(dig_t, &settings.DigitalHour);
  settings_changed |= prv_set_bool(qt_blank_face_t, &settings.QuietTimeBlankFace);
  settings_changed |= prv_set_bool(blank_face_t, &settings.BlankFaceMode);
  settings_changed |= prv_set_int32(ocent_t, (int*)&settings.CentreSize);
  settings_changed |= prv_set_int32(icent_t, &settings.InnerCentreSize);
  settings_changed |= prv_set_int32(hand_t, &settings.HandThickness);
  settings_changed |= prv_set_int32(back_t, &settings.BackSize);
  settings_changed |= prv_set_int32(backlen_t, &settings.BackLen);
  settings_changed |= prv_set_bool(majort_t, &settings.showMajorTick);
  settings_changed |= prv_set_bool(minort_t, &settings.showMinorTick);

  if(posleft_t){
    snprintf(settings.PosLeft, sizeof(settings.PosLeft), "%s", posleft_t -> value -> cstring);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if(posright_t){
    snprintf(settings.PosRight, sizeof(settings.PosRight), "%s", posright_t -> value -> cstring);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if(postop_t){
    snprintf(settings.PosTop, sizeof(settings.PosTop), "%s", postop_t -> value -> cstring);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if(posbottom_t){
    snprintf(settings.PosBottom, sizeof(settings.PosBottom), "%s", posbottom_t -> value -> cstring);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  settings_changed |= prv_set_bool(vibe_t, &settings.VibeOn);
  settings_changed |= prv_set_bool(addzero12_t, &settings.AddZero12h);
  settings_changed |= prv_set_bool(remzero24_t, &settings.RemoveZero24h);
  settings_changed |= prv_set_bool(enable_date_t, &settings.EnableDate);

  if (enable_logo_t) {
    settings.EnableLogo = enable_logo_t->value->int32 == 1;

    // Check if the logo is enabled and the custom text string is not empty
    if (settings.EnableLogo && logotext_t && strlen(logotext_t->value->cstring) > 0) {
      // If the custom text field is not blank, use the user's text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", logotext_t->value->cstring);
    } else if (settings.EnableLogo && strlen(logotext_t->value->cstring) == 0) {
      // If the custom text field is blank but the logo is enabled, use the default text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "pebble");
    }
    else {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "");
    }

    settings_changed = true;
  }

  settings_changed |= prv_set_bool(enable_battery_t, &settings.EnableBattery);
  settings_changed |= prv_set_bool(enable_battery_line_t, &settings.EnableBatteryLine);
  settings_changed |= prv_set_int32(complication_font_size_adj_t, &settings.ComplicationFontSizeAdj);

  if (minute_hand_updates_per_min_t) {
    int updates = minute_hand_updates_per_min_t->value->int32;
    settings.SmoothMinuteHand = settings.DigitalHour && updates > 1;
    settings.MinuteHandUpdateIntervalSec = (60 + updates / 2) / updates;

#if defined(SUB_MINUTE_USE_APPTIMER)
    if (settings.SmoothMinuteHand) {
      sub_minute_timer = app_timer_register(1000 * settings.MinuteHandUpdateIntervalSec, apptimer_handler, NULL);
    }
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
#elif defined(SUB_MINUTE_USE_TICK)
    tick_timer_service_subscribe((use_minute_hand() && settings.SmoothMinuteHand) ?
      SECOND_UNIT : MINUTE_UNIT, tick_handler);
#endif

    settings_changed = true;
  }

  settings_changed |= prv_set_bool(orbit_complications_t, &settings.OrbitComplications);

  if (enable_alarm_calendar_sync_t) {
    settings.EnableAlarmCalendarSync = enable_alarm_calendar_sync_t->value->int32 == 1;
    alarm_calendar_sync_set_enabled(settings.EnableAlarmCalendarSync);
  }

  if (timeline_alarm_pin_t) {
    settings.TimelineAlarmPin = timeline_alarm_pin_t->value->int32 == 1;
    alarm_calendar_sync_set_alarm_pin(settings.TimelineAlarmPin);
  }

  if (timeline_timer_pin_t) {
    settings.TimelineTimerPin = timeline_timer_pin_t->value->int32 == 1;
    alarm_calendar_sync_set_timer_pin(settings.TimelineTimerPin);
  }

  settings_changed |= prv_set_color(local_alarm_pin_color_t, &settings.LocalAlarmPinColor);
  settings_changed |= prv_set_color(synced_alarm_pin_color_t, &settings.SyncedAlarmPinColor);
  settings_changed |= prv_set_color(calendar_pin_color_t, &settings.CalendarPinColor);

  if (show_watch_dial_window_t) {
    settings.ShowWatchDialWindow = (show_watch_dial_window_t->value->int32 == 1) && settings.OrbitComplications;
    #ifdef PBL_RECT
    settings.ShowWatchDialWindow = settings.ShowWatchDialWindow && settings.ForegroundShape;
    #endif
    settings_changed = true;
  }

  settings_changed |= prv_set_color(watch_dial_window_color_t, &settings.WatchDialWindowColor);

  // Force the hand to switch between hours and minutes
  if (oldUseMinuteHand != use_minute_hand())
    hand_angle_native = calculate_hand_angle(prv_tick_time);

  /////////////////////////////////////
  // Set the colors for the watchface
  settings_changed |= prv_set_color(bg_color_t, &settings.BackgroundColor);
  settings_changed |= prv_set_color(comp_border_color_t, &settings.ComplicationBorderColor);
  settings_changed |= prv_set_color(comp_background_color_t, &settings.ComplicationBackgroundColor);
  settings_changed |= prv_set_color(comp_shadow_color_t, &settings.ComplicationShadowColor);

  if (shadowon_t) {
    settings.ShadowOn = shadowon_t->value->int32 == 1;

      if(settings.ShadowOn){
        settings_changed |= prv_set_color(bg_color2_t, &settings.MinuteHandShadowColor);
      }
      else {
      settings.MinuteHandShadowColor = settings.BackgroundColor;
      }
  }

  settings_changed |= prv_set_color(text_color2_t, &settings.MinorTickColor);
  settings_changed |= prv_set_color(date_color_t, &settings.DateColor);
  settings_changed |= prv_set_color(hours_digits_color_t, &settings.HourDigitsColor);
  settings_changed |= prv_set_color(minute_digits_color_t, &settings.MinuteDigitsColor);
  settings_changed |= prv_set_color(minutes_hand_color_t, &settings.MinutesHandColor);
  settings_changed |= prv_set_color(hour_hand_color_t, &settings.HourHandColor);
  settings_changed |= prv_set_color(tick_color_t, &settings.MajorTickColor);
  settings_changed |= prv_set_color(minimized_major_tick_color_t, &settings.MinimizedMajorTickColor);
  settings_changed |= prv_set_color(battery_line_color_t, &settings.BatteryLineColor);
  settings_changed |= prv_set_color(btqt_color_t, &settings.BTQTColor);

  ///////////////////////////////

  if (settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
    layer_mark_dirty(s_alarm_cal_pin_layer);
  }

  prv_save_settings();
}

#if defined(SUB_MINUTE_USE_APPTIMER)
static void apptimer_handler(void *data) {
  sub_minute_timer = NULL;
  time_t tm = time(NULL);
  tick_handler(localtime(&tm), SECOND_UNIT);
}
#endif

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler fired: %02d:%02d:%02d", tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);

  memcpy(prv_tick_time, tick_time, sizeof(struct tm));
  bool minute_changed = units_changed & MINUTE_UNIT;

#if defined(SUB_MINUTE_USE_TICK)
  bool process_sub_min_tick = false;
  if (settings.SmoothMinuteHand && units_changed & SECOND_UNIT) {
    int new_interval = prv_tick_time->tm_sec / settings.MinuteHandUpdateIntervalSec;
    process_sub_min_tick = new_interval != sub_minute_interval;
  }
#elif defined(SUB_MINUTE_USE_APPTIMER)
  bool process_sub_min_tick = settings.SmoothMinuteHand && units_changed & SECOND_UNIT;
#endif

  // Update hour and minute hands and the date on minute change
  if (minute_changed || process_sub_min_tick) {
    seconds = tick_time->tm_sec;
    #if defined(SUB_MINUTE_USE_TICK)
    sub_minute_interval = tick_time->tm_sec / settings.MinuteHandUpdateIntervalSec;
    #endif
    minutes = tick_time->tm_min;
    hours = tick_time->tm_hour % 12;
    s_hours = tick_time->tm_hour;

    #ifdef SHOW_MINUTE
    prv_tm.tm_min = minutes = SHOW_MINUTE;
    #endif

    hand_angle_native = calculate_hand_angle(prv_tick_time);

    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
    layer_mark_dirty(s_alarm_cal_pin_layer);

    if (settings.EnableDate && tick_time->tm_mday != current_date) {
      current_date = tick_time->tm_mday;
      s_weekday = tick_time->tm_wday;
    }

    #if defined(SUB_MINUTE_USE_APPTIMER)
    if (settings.SmoothMinuteHand && (seconds + settings.MinuteHandUpdateIntervalSec) < 60) {
      // Schedule a timer for smooth minute hand movement unless the next callback is a minute tick
      sub_minute_timer = app_timer_register(1000 * settings.MinuteHandUpdateIntervalSec, apptimer_handler, NULL);
    }
    #endif

    // Periodically check whether the alarm/calendar data needs a re-sync.
    if (minute_changed)
      alarm_calendar_sync_maybe_request_update();
  }
}

///analogue hand
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint origin_offset = GPoint(origin.x + config.hands_shadow, origin.y + config.hands_shadow);
  GPoint p1;
  GPoint p2;
  GPoint p3;
  GPoint p4;
  
  #ifdef PBL_ROUND
      p1 = polar_to_point_offset_native(origin, angle + TRIG_HALF_ANGLE, back_length);
      p2 = polar_to_point_offset_native(origin, angle, length);
      p3 = polar_to_point_offset_native(origin_offset, angle + TRIG_HALF_ANGLE, back_length);
      p4 = polar_to_point_offset_native(origin_offset, angle, length);
  #else
    if(settings.ForegroundShape){
      p1 = polar_to_point_offset_native(origin, angle + TRIG_HALF_ANGLE, back_length);
      p2 = polar_to_point_offset_native(origin, angle, length);
      p3 = polar_to_point_offset_native(origin_offset, angle + TRIG_HALF_ANGLE, back_length);
      p4 = polar_to_point_offset_native(origin_offset, angle, length);
    }
    else{
      p1 = polar_to_point_offset_native(origin, angle + TRIG_HALF_ANGLE, back_length);
      p2 = angle_to_rounded_rect_edge_native(origin, angle, bounds.size.w/2-10, bounds.size.h/2-10, config.corner_radius_secondshand);
      p3 = polar_to_point_offset_native(origin_offset, angle + TRIG_HALF_ANGLE, back_length);
      p4 = angle_to_rounded_rect_edge_native(origin_offset, angle, bounds.size.w/2-10, bounds.size.h/2-10, config.corner_radius_secondshand);

    }
  #endif
  // Define shadow color
  GColor shadow_color = settings.MinuteHandShadowColor;


  // Set the antialiasing
  graphics_context_set_antialiased(ctx, true);

  // Draw the shadow first, with a small offset
  graphics_context_set_stroke_color(ctx, shadow_color);
  graphics_context_set_fill_color(ctx, shadow_color);
  graphics_context_set_stroke_width(ctx, settings.HandThickness); // Same width as the hand
  graphics_draw_line(ctx, GPoint(p3.x, p3.y), GPoint(p4.x, p4.y));

  GPoint origin_back_offset = GPoint(p1.x + config.hands_shadow, p1.y + config.hands_shadow);
  graphics_fill_circle(ctx, origin_back_offset, settings.BackSize);
  graphics_fill_circle(ctx, origin_offset, settings.CentreSize); //started as 4

  // Now draw the main hand on top
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, settings.HandThickness);
  graphics_draw_line(ctx, p1, p2);

  graphics_context_set_fill_color(ctx, color);
  GPoint origin_back = GPoint(p1.x, p1.y);
  graphics_fill_circle(ctx, origin_back, settings.BackSize);

}

static void draw_hand_center(GContext *ctx, GColor outer_color, GColor inner_color) {
  if (settings.CentreSize == 0 && settings.InnerCentreSize == 0)
    return;

  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, outer_color);
  graphics_fill_circle(ctx, origin, settings.CentreSize); //started as 4
  
  graphics_context_set_fill_color(ctx, inner_color);
  graphics_fill_circle(ctx, origin, settings.InnerCentreSize); //started as 2

}

static void draw_event_pin(GContext *ctx, int hour, int minute, int second, GColor color) {
  static const int pin_length = 13; // Halfway between major and minor tick lengths with room for an outline
  static const int pin_half_angle = DEG_TO_TRIGANGLE(35); // Half of the angle for the pin's width

  int angle_native = use_minute_hand() ?
    (TRIG_MAX_ANGLE * minute / 60) + (TRIG_MAX_ANGLE * second / 3600) - TRIG_QUARTER_ANGLE :
    (TRIG_MAX_ANGLE * hour / 12) + (TRIG_MAX_ANGLE * minute / 720) + (TRIG_MAX_ANGLE * second / (12 * 60 * 60)) - TRIG_QUARTER_ANGLE;
  int pin_center_angle = angle_native - TRIG_QUARTER_ANGLE;
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint edge;

#ifdef PBL_ROUND
  edge = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2);
#else
  if (settings.ForegroundShape) {
    edge = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2);
  } else {
    GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
    edge = angle_to_rect_edge_native(origin, angle_native, r);
  }
#endif

  GRect pin_rect = GRect(edge.x - pin_length, edge.y - pin_length, pin_length * 2, pin_length * 2);

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_radial(ctx, pin_rect, GOvalScaleModeFitCircle, pin_length,
     pin_center_angle - pin_half_angle, pin_center_angle + pin_half_angle);

  // Draw contrasting highlights around the pin
  int adj_pin_angle = pin_center_angle - TRIG_QUARTER_ANGLE;
  graphics_context_set_stroke_color(ctx, get_contrasting_color(color));
  graphics_context_set_stroke_width(ctx, 1);
  GPoint highlightpt = polar_to_point_offset_native(edge, adj_pin_angle - pin_half_angle, pin_length);
  graphics_draw_line(ctx, edge, highlightpt);
  highlightpt = polar_to_point_offset_native(edge, adj_pin_angle + pin_half_angle, pin_length);
  graphics_draw_line(ctx, edge, highlightpt);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_arc(ctx, pin_rect, GOvalScaleModeFitCircle, pin_center_angle - pin_half_angle, pin_center_angle + pin_half_angle);
}

static void draw_event_arc(GContext *ctx, int hour1, int minute1, int second1, int hour2, int minute2, int second2, GColor color) {
  static const int arc_width = 5;

  if (hour1 == hour2 && minute1 == minute2 && second1 == second2)
    return;

  int arc_start_angle = use_minute_hand() ?
    (TRIG_MAX_ANGLE * minute1 / 60) + (TRIG_MAX_ANGLE * second1 / 3600) :
    (TRIG_MAX_ANGLE * hour1 / 12) + (TRIG_MAX_ANGLE * minute1 / 720);
  int arc_end_angle = use_minute_hand() ?
    (TRIG_MAX_ANGLE * minute2 / 60) + (TRIG_MAX_ANGLE * second2 / 3600) :
    (TRIG_MAX_ANGLE * hour2 / 12) + (TRIG_MAX_ANGLE * minute2 / 720);
  if (arc_end_angle <= arc_start_angle)
    arc_end_angle += TRIG_MAX_ANGLE;

  APP_LOG(APP_LOG_LEVEL_INFO, "drawcalendar: start_angle %08x, end_angle %08x", arc_start_angle, arc_end_angle);

  GRect arc_bounds = GRect(arc_width, arc_width, bounds.size.w - 2 * arc_width, bounds.size.h - 2 * arc_width);

  // Draw a filled arc, then a line of a contrasting color
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, get_contrasting_color(color));
  graphics_context_set_stroke_width(ctx, 1);

  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, arc_width, arc_start_angle, arc_end_angle);
  graphics_draw_arc(ctx, arc_bounds, GOvalScaleModeFitCircle, arc_start_angle, arc_end_angle);
}

static void layer_update_proc_alarm_cal_pins(Layer *layer, GContext *ctx) {
  // When using minute hand, 1 hour, otherwise, 12 hours
  uint32_t timeThresholdSec = use_minute_hand() ? 60 * 60 : 12 * 60 * 60;
  time_t now = time(NULL);

  // Draw local alarm pin if it's going off within the next hour
  if (next_alarm_time) {
    uint32_t diff = next_alarm_time - now;
    if (diff < timeThresholdSec) {
      struct tm *lalarm_tm = localtime(&next_alarm_time);
      draw_event_pin(ctx, lalarm_tm->tm_hour, lalarm_tm->tm_min, 0, settings.LocalAlarmPinColor);
    }
  }

  // Only draw pins for remotely synced items when the sync feature is enabled.
  if (!settings.EnableAlarmCalendarSync) {
    return;
  }

  // Draw alarm pin (if set and within the next ~hour).
  uint32_t alarm_epoch = alarm_calendar_sync_get_alarm();
  if (alarm_epoch != 0) {
    uint32_t diff = alarm_epoch - now;
    if (diff < timeThresholdSec) {
      time_t t = (time_t)alarm_epoch;
      struct tm *alarm_tm = localtime(&t);
      draw_event_pin(ctx, alarm_tm->tm_hour, alarm_tm->tm_min, 0, settings.SyncedAlarmPinColor);
    }
  }

  // Draw synced timer pin (if running and within the next ~hour), same colour
  // as the alarm pin.
  uint32_t timer_epoch = alarm_calendar_sync_get_timer();
  if (timer_epoch != 0) {
    uint32_t diff = timer_epoch - now;
    if (diff < timeThresholdSec) {
      time_t t = (time_t)timer_epoch;
      struct tm *timer_tm = localtime(&t);
      draw_event_pin(ctx, timer_tm->tm_hour, timer_tm->tm_min, timer_tm->tm_sec, settings.SyncedAlarmPinColor);
    }
  }

  // Draw calendar event pins (if any and within the next ~hour).
  int count = alarm_calendar_sync_get_event_count();
  time_t thresholdTime = now + timeThresholdSec;
  for (int i = 0; i < count; i++) {
    uint32_t diff;
    CalendarEvent event = alarm_calendar_sync_get_event_at(i);
    if (event.start_epoch == 0) {
      continue;
    }

    bool single_point = event.end_epoch == event.start_epoch;
    bool draw_start = (event.start_epoch - now) < timeThresholdSec;
    bool draw_end = !single_point && (event.end_epoch - now) < timeThresholdSec;
    bool draw_arc = !single_point && ((time_t)event.start_epoch < thresholdTime) && ((time_t)event.end_epoch >= now);
    APP_LOG(APP_LOG_LEVEL_INFO, "drawcalendar: start %d, end %d, arc %d", draw_start, draw_end, draw_arc);

    if (draw_arc) {
      time_t start_t = ((time_t)event.start_epoch > now) ? (time_t)event.start_epoch : now;
      time_t end_t = ((time_t)event.end_epoch < thresholdTime) ? (time_t)event.end_epoch : thresholdTime;

      APP_LOG(APP_LOG_LEVEL_INFO, "drawcalendar: start_t %d, end_t %d", start_t, end_t);

      struct tm *event_tm = localtime(&start_t);
      int start_hr = event_tm->tm_hour;
      int start_min = event_tm->tm_min;
      int start_sec = event_tm->tm_sec;
      event_tm = localtime(&end_t);
      draw_event_arc(ctx, start_hr, start_min, start_sec, event_tm->tm_hour, event_tm->tm_min, event_tm->tm_sec, settings.CalendarPinColor);
    }

    if (draw_end) {
      diff = event.end_epoch - now;
      if (diff < timeThresholdSec) {
        time_t t = (time_t)event.end_epoch;
        struct tm *event_tm = localtime(&t);
        draw_event_pin(ctx, event_tm->tm_hour, event_tm->tm_min, 0, settings.CalendarPinColor);
      }
    }

    if (draw_start) {
      diff = event.start_epoch - now;
      if (diff < timeThresholdSec) {
        time_t t = (time_t)event.start_epoch;
        struct tm *event_tm = localtime(&t);
        draw_event_pin(ctx, event_tm->tm_hour, event_tm->tm_min, 0, settings.CalendarPinColor);
      }
    }
  }
}

static void draw_radial_line(GContext *ctx, int angle_native, int length, GColor border_color) {
  // Pebble SDK draw angles are offset by 90 degrees compared to ours, so keep it consistent here
  angle_native -= TRIG_QUARTER_ANGLE;
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 );
  GPoint p2 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 - length);
  graphics_draw_line(ctx, p1, p2);
}

static void draw_major_tick (GContext *ctx, int angle_native, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
      GPoint p1;
      GPoint p2;

      #ifdef PBL_ROUND
        p1 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 );
        p2 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 - length);
      #else
        if(settings.ForegroundShape){
          p1 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 );
          p2 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 - length);
        }
        else{
          GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
          GPoint edge = angle_to_rect_edge_native(origin, angle_native, r);
          int32_t dx = cos_lookup(angle_native);
          int32_t dy = sin_lookup(angle_native);
          p2 = GPoint(edge.x - (int)((dx * config.tick_inset_outer) / TRIG_MAX_ANGLE),
                            edge.y - (int)((dy * config.tick_inset_outer) / TRIG_MAX_ANGLE));
          p1 = angle_to_rounded_rect_edge_native(origin, angle_native, config.majortickrect_w, config.majortickrect_h, config.corner_radius_majortickrect);
        }
      #endif
 
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, p1, p2);
}

static void draw_minor_tick(GContext *ctx, int angle_native, GColor border_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1;
  GPoint p2;

  #ifdef PBL_ROUND
    // The tick starts away from the center of the watch face.
    p1 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 - 8);
    // The tick ends closer to the edge.
    p2 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 );
  #else
    if (settings.ForegroundShape) {
      p1 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 - 8);
      p2 = polar_to_point_offset_native(origin, angle_native, bounds.size.h / 2 );
    } else {
      GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
      GPoint edge = angle_to_rect_edge_native(origin, angle_native, r);
      int32_t dx = cos_lookup(angle_native);
      int32_t dy = sin_lookup(angle_native);
      p2 = GPoint(edge.x - (int)((dx * config.tick_inset_outer) / TRIG_MAX_ANGLE),
                        edge.y - (int)((dy * config.tick_inset_outer) / TRIG_MAX_ANGLE));
      p1 = angle_to_rounded_rect_edge_native(origin, angle_native, config.minortickrect_w, config.minortickrect_h, config.corner_radius_minortickrect);
    }
  #endif

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 1); // A thin line for minor ticks
  graphics_draw_line(ctx, p1, p2);
}


// fctx based rendering

static FPoint gpoint_to_fpoint(GPoint *gpoint) {
  return
    (FPoint){ .x = INT_TO_FIXED(gpoint->x),
              .y = INT_TO_FIXED(gpoint->y) };
}

static GPoint relative_gpoint_to_absolute(GPoint *gpoint) {
  return
    (GPoint){ .x = bounds.size.w / 2 + gpoint->x,
              .y = bounds.size.h / 2 + gpoint->y };
}

static void draw_complication_border_bg(FContext *fctxp, GPoint center) {
  bool draw_border = settings.ComplicationBorderColor.argb != settings.BackgroundColor.argb;
  bool draw_background = settings.ComplicationBackgroundColor.argb != settings.BackgroundColor.argb;

  if (!draw_border && !draw_background)
    return;

  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  int dialWindowAdj = settings.ShowWatchDialWindow ? config.ComplicationDialWindowSizeAdj : 0;
  int radius = 5 * bounds.size.w / 32 + config.ComplicationBorderAdj + orbitadj + dialWindowAdj;
  graphics_context_set_antialiased(fctxp->gctx, true);

  if (draw_background) {
    graphics_context_set_fill_color(fctxp->gctx, settings.ComplicationBackgroundColor);
    graphics_fill_circle(fctxp->gctx, center, radius);
  }

  if (draw_border) {
    graphics_context_set_stroke_color(fctxp->gctx, settings.ComplicationBorderColor);
    graphics_context_set_stroke_width(fctxp->gctx, 1);

    graphics_draw_circle(fctxp->gctx, center, radius);

    if (settings.ShadowOn && settings.ComplicationShadowColor.argb != settings.BackgroundColor.argb) {
      GRect shadow_rect = GRect(center.x - radius, center.y - radius + 1, 2 * radius, 2 * radius);
      graphics_context_set_stroke_color(fctxp->gctx, settings.ComplicationShadowColor);
      graphics_draw_arc(fctxp->gctx, shadow_rect, GOvalScaleModeFitCircle, TRIG_QUARTER_ANGLE, TRIG_HALF_ANGLE + TRIG_QUARTER_ANGLE);
    }
  }
}

static void render_btqt_fctx(FContext *fctxp, FPoint icons_bottom) {
  char status_string[6];
  char *status = status_string;

  if (quiet_time_is_active())
    status += snprintf(status_string, sizeof(status_string), "\U0000E061");
  if (!connection_service_peek_pebble_app_connection())
    *(status++) = 'z';
  if (status_string == status)
    return;
  *status = '\0';

  int font_size = config.font_size_btqt;
  fctx_set_fill_color(fctxp, settings.BTQTColor);
  fctx_begin_fill(fctxp);
  fctx_set_text_em_height(fctxp, FontBTQTIconsFctx, font_size);
  fctx_set_offset(fctxp, icons_bottom);
  fctx_draw_string(fctxp, status_string, FontBTQTIconsFctx, GTextAlignmentCenter, FTextAnchorBottom);
  fctx_end_fill(fctxp);
}

static GPoint get_complication_pos(int angle_native) {
  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  int dialWindowAdj = settings.ShowWatchDialWindow ? config.ComplicationDialWindowDistanceAdj : 0;
  GPoint rel_pos = polar_to_point_native(angle_native, bounds.size.w / 4 + config.ComplicationDistanceAdj - orbitadj + dialWindowAdj);
  return relative_gpoint_to_absolute(&rel_pos);
}

static void render_hour_digits_fctx(FContext *fctxp, int angle_native) {
  fctx_set_fill_color(fctxp, settings.HourDigitsColor);
  GPoint abs_pos = get_complication_pos(angle_native);
  FPoint hour_pos = gpoint_to_fpoint(&abs_pos);

  draw_complication_border_bg(fctxp, abs_pos);

  char mindraw[3];
  snprintf(mindraw, sizeof(mindraw), "%02d", minutes);

  int hourdraw;
    char hournow[4];
    if (clock_is_24h_style()) {
      hourdraw = s_hours;
      snprintf(hournow, sizeof(hournow), settings.RemoveZero24h ? "%d" : "%02d", hourdraw);
    } else {
      if (s_hours == 0 || s_hours == 12) hourdraw = 12;
      else 
      hourdraw = s_hours % 12;
      snprintf(hournow, sizeof(hournow), settings.AddZero12h ? "%02d" : "%d", hourdraw);
    }

  fctx_begin_fill(fctxp);
  fctx_set_text_em_height(fctxp, Date_Font, config.font_size_digits);

  fctx_set_offset(fctxp, hour_pos);
  fctx_set_offset(fctxp, hour_pos);
  if(use_minute_hand()){
    fctx_draw_string(fctxp, hournow, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
  }
  else{
    fctx_draw_string(fctxp, mindraw, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
  }
  fctx_end_fill(fctxp);

  hour_pos.y -= INT_TO_FIXED(config.font_size_digits / 2 - 4);
  render_btqt_fctx(fctxp, hour_pos);
}

static void render_ampm_fctx(FContext *fctxp, int angle_native) {
  fctx_set_fill_color(fctxp, settings.HourDigitsColor);

  GPoint abs_pos = get_complication_pos(angle_native);
  FPoint ampm_pos = gpoint_to_fpoint(&abs_pos);

  draw_complication_border_bg(fctxp, abs_pos);

  char local_ampm_string[5];
  strftime(local_ampm_string, sizeof(local_ampm_string), "%p", prv_tick_time);

  fctx_begin_fill(fctxp);
  fctx_set_text_em_height(fctxp, Date_Font, config.font_size_battery + settings.ComplicationFontSizeAdj);
        fctx_set_offset(fctxp, ampm_pos);
        fctx_draw_string(fctxp, local_ampm_string, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
        fctx_end_fill(fctxp);
}

static void render_logo_fctx(FContext *fctxp, FPoint render_pos) {
  #ifdef PBL_PLATFORM_EMERY
    #define LOGO_WRAP_AT 8
  #else //if defined (PBL_PLATFORM_GABBRO)
    #define LOGO_WRAP_AT 12
  #endif

  const int logo_top_margin = 2;
  const int logo_line_spacing = 2;
  fctx_set_fill_color(fctxp, settings.DateColor);

  render_pos.y += INT_TO_FIXED(logo_line_spacing + logo_top_margin);
  int font_size_logo = config.font_size_logo + settings.ComplicationFontSizeAdj;
  char logodraw[20];

  snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);
  char *line2 = NULL;
  if (strlen(logodraw) > LOGO_WRAP_AT) {
      char *split = NULL;
      for (int i = LOGO_WRAP_AT; i >= 0; i--) {
          if (logodraw[i] == ' ') { split = &logodraw[i]; break; }
      }
      if (split) { *split = '\0'; line2 = split + 1; }
  }
  if (line2) {
      fctx_begin_fill(fctxp);
      fctx_set_text_em_height(fctxp, Date_Font, font_size_logo);
      fctx_set_offset(fctxp, render_pos);
      fctx_draw_string(fctxp, logodraw, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
      fctx_end_fill(fctxp);
      render_pos.y += INT_TO_FIXED(font_size_logo + logo_line_spacing);
      fctx_begin_fill(fctxp);
      fctx_set_text_em_height(fctxp, Date_Font, font_size_logo);
      fctx_set_offset(fctxp, render_pos);
      fctx_draw_string(fctxp, line2, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
      fctx_end_fill(fctxp);
  } else {
      fctx_begin_fill(fctxp);
      fctx_set_text_em_height(fctxp, Date_Font, font_size_logo);
      fctx_set_offset(fctxp, render_pos);
      fctx_draw_string(fctxp, logodraw, Date_Font, GTextAlignmentCenter, FTextAnchorTop);
      fctx_end_fill(fctxp);
  }
}

static void render_battery_pct_fctx(FContext *fctxp, FPoint render_pos) {
  const int battery_line_offset = 2;
  const int battery_pct_offset = 2;

  fctx_set_fill_color(fctxp, settings.DateColor);

  int font_size_battery = config.font_size_battery + settings.ComplicationFontSizeAdj;
  
  int s_battery_level = battery_state_service_peek().charge_percent;
  fctx_begin_fill(fctxp);
  fctx_set_text_em_height(fctxp, Date_Font, font_size_battery);

  render_pos.y -= battery_pct_offset;
  if (settings.EnableBatteryLine)
    render_pos.y -= battery_line_offset;

  char BatterytoDraw[6];
  snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

  fctx_set_offset(fctxp, render_pos);
  fctx_draw_string(fctxp, BatterytoDraw, Date_Font, GTextAlignmentCenter, FTextAnchorBottom);
  fctx_end_fill(fctxp);
}

static void render_logo_battery_fctx(FContext *fctxp, int angle_native) {
  FPoint render_pos;
  if (settings.EnableBattery || settings.EnableLogo || settings.EnableBatteryLine) {
    GPoint abs_pos = get_complication_pos(angle_native);
    render_pos = gpoint_to_fpoint(&abs_pos);

    draw_complication_border_bg(fctxp, abs_pos);
  }

  if (settings.EnableBattery)
    render_battery_pct_fctx(fctxp, render_pos);
  if (settings.EnableLogo)
    render_logo_fctx(fctxp, render_pos);
}

static void render_date_fctx(FContext *fctxp, int angle_native) {
  fctx_set_fill_color(fctxp, settings.DateColor);

  GPoint abs_pos = get_complication_pos(angle_native);
  FPoint weekday_pos = gpoint_to_fpoint(&abs_pos);
  FPoint date_pos = weekday_pos;

  draw_complication_border_bg(fctxp, abs_pos);

  int font_size_date = config.font_size_date + settings.ComplicationFontSizeAdj;

  weekday_pos.y -= INT_TO_FIXED(font_size_date / 2 + 1);
  date_pos.y += INT_TO_FIXED(font_size_date / 2 + 1);

  fctx_begin_fill(fctxp);
  fctx_set_text_em_height(fctxp, Date_Font, font_size_date);

  const char * sys_locale = i18n_get_system_locale();
  char weekday[5];
  fetchwday(s_weekday, sys_locale, weekday);

  char weekdaydraw[10];
  snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

  fctx_set_offset(fctxp, weekday_pos);
  fctx_draw_string(fctxp, weekdaydraw, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
  fctx_end_fill(fctxp);

  fctx_begin_fill(fctxp);
  fctx_set_text_em_height(fctxp, Date_Font, font_size_date);

  char daynow[5];
  snprintf(daynow, sizeof(daynow), "%d", current_date);

  fctx_set_offset(fctxp, date_pos);
  fctx_draw_string(fctxp, daynow, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
  fctx_end_fill(fctxp);
}

static void render_battery_line(GContext *ctx, int angle_native, int battery_level) {
  int width_rect = (battery_level * config.battery_line) / 100;
  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  int dialWindowAdj = settings.ShowWatchDialWindow ? config.ComplicationDialWindowDistanceAdj : 0;
  GPoint line_center = polar_to_point_native(angle_native, bounds.size.w/4 + config.ComplicationDistanceAdj - orbitadj + dialWindowAdj);
  line_center.x += bounds.size.w / 2;
  line_center.y += bounds.size.h / 2;

  GRect BatteryLineRect = GRect(line_center.x - width_rect/2, line_center.y, width_rect, 2);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, settings.BatteryLineColor);
  graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);  
}

static inline int get_base_angle() {
  const int top_angle = -TRIG_QUARTER_ANGLE;
  return settings.OrbitComplications ? hand_angle_native : top_angle;
}

static void update_logo_date_battery_fctx_layer (Layer *layer, GContext *ctx) {
  if (skip_render_complications())
    return;

  int base_angle = get_base_angle();

  //APP_LOG(APP_LOG_LEVEL_INFO, "update_logo_date_battery_fctx_layer");
  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  #ifdef PBL_COLOR
   fctx_enable_aa(true);
  #endif

  int startidx = settings.OrbitComplications ? 1 : 0;
  char* compSettings[] = { settings.PosTop, settings.PosRight, settings.PosBottom, settings.PosLeft, NULL };
  int side_angle = settings.OrbitComplications ? TRIG_7_32_ANGLE : TRIG_QUARTER_ANGLE;
  int angles[] = { base_angle, base_angle + side_angle, base_angle + TRIG_HALF_ANGLE, base_angle - side_angle, 0 };
  int *curr_angle = &angles[startidx];
  for (char** setting = compSettings + startidx;
      *setting;
      curr_angle++, setting++) {

    char* currSetting = *setting;

    if (strncmp(currSetting, "hr", 3) == 0) {
      render_hour_digits_fctx(&fctx, *curr_angle);
    } else if(strncmp(currSetting, "lo", 3) == 0) {
      render_logo_battery_fctx(&fctx, *curr_angle);
      if (settings.EnableBatteryLine) {
        render_battery_line(fctx.gctx, *curr_angle, battery_state_service_peek().charge_percent);
      }
    } else if (strncmp(currSetting, "dt", 3) == 0) {
      if (settings.EnableDate)
        render_date_fctx(&fctx, *curr_angle);
    } else if (strncmp(currSetting, "ap", 3) == 0) {
      if (!clock_is_24h_style())
        render_ampm_fctx(&fctx, *curr_angle);
    }
  }

  fctx_deinit_context(&fctx);
}

int calculate_hand_angle(struct tm *prv_tm) {
  // Using native trig angles since we're dealing with small fractions of degrees
  int angle;

  if (use_minute_hand()) {
    angle = (TRIG_MAX_ANGLE * prv_tm->tm_min / 60) - TRIG_QUARTER_ANGLE;
    
    if (settings.SmoothMinuteHand) {
      angle += TRIG_MAX_ANGLE * prv_tm->tm_sec / 60 / 60;  // Sweep 1/60 of a circle over 60s
    }
  } else {
    angle = (TRIG_MAX_ANGLE * (prv_tm->tm_hour % 12) / 12) + (TRIG_MAX_ANGLE * prv_tm->tm_min / 60 / 12) - TRIG_QUARTER_ANGLE;
  }

  return angle;
}

// Update procedure for the main canvas layer (hour & minute hands)
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  #ifdef PBL_ROUND
      draw_line_hand(ctx, hand_angle_native,
          bounds.size.w/2 - config.analogue_hand_a,
          settings.BackLen,
          settings.MinutesHandColor);
      draw_hand_center(ctx, settings.MinutesHandColor, settings.BackgroundColor);
  #else
      if(settings.ForegroundShape){
          draw_line_hand(ctx, hand_angle_native,
              bounds.size.w/2 - config.analogue_hand_a,
              settings.BackLen,
              settings.MinutesHandColor);
          draw_hand_center(ctx, settings.MinutesHandColor, settings.BackgroundColor);
      }
      else{
          draw_line_hand(ctx, hand_angle_native,
              bounds.size.w/2 - config.analogue_hand_c,
              settings.BackLen,
              settings.MinutesHandColor);
          draw_hand_center(ctx, settings.MinutesHandColor, settings.BackgroundColor);
      }
  #endif

}

#define DIAL_WINDOW_SWEEP_ANGLE TRIG_7_32_ANGLE
// Max number of minutes swept by 7/32 of a circle, rounded up
#define DIAL_WINDOW_SWEEP_MINUTES ((60 + 7) + 31 / 32)

///update procedure for background
static void bg_update_proc(Layer *layer, GContext *ctx) {

  GRect bounds = layer_get_bounds(layer);

  GRect Background = GRect(0, 0, bounds.size.w, bounds.size.h);
  int window_start_angle = 0, window_end_angle = 0, window_thickness = 0;

  graphics_context_set_fill_color(ctx, settings.BackgroundColor);
  graphics_fill_rect(ctx, Background,0,GCornersAll);

  if (settings.ShowWatchDialWindow && (settings.showMinorTick || settings.showMajorTick)) {
    window_start_angle = hand_angle_native - (DIAL_WINDOW_SWEEP_ANGLE / 2) + TRIG_QUARTER_ANGLE;
    window_end_angle = hand_angle_native + (DIAL_WINDOW_SWEEP_ANGLE / 2) + TRIG_QUARTER_ANGLE;
    window_thickness = bounds.size.h * 5 / 32 + 1;

    // Draw the actual window and outline
    graphics_context_set_fill_color(ctx, settings.WatchDialWindowColor);
    graphics_fill_radial(ctx, Background, GOvalScaleModeFillCircle, window_thickness, window_start_angle, window_end_angle);

    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_stroke_color(ctx, settings.ComplicationShadowColor);
    draw_radial_line(ctx, window_start_angle, window_thickness - 1, settings.ComplicationShadowColor);
    draw_radial_line(ctx, window_end_angle, window_thickness - 1, settings.ComplicationShadowColor);

    GRect window_border_arc = GRect(Background.origin.x + window_thickness, Background.origin.y + window_thickness, Background.size.w - 2 * window_thickness, Background.size.h - 2 * window_thickness);
    graphics_draw_arc(ctx, window_border_arc, GOvalScaleModeFillCircle, window_start_angle, window_end_angle);
    #if PBL_RECT
    graphics_draw_arc(ctx, Background, GOvalScaleModeFillCircle, window_start_angle, window_end_angle);
    #endif
  }

  if (settings.showMinorTick) {
    // 4 ticks per hour with an hour hand
    int num_ticks = use_minute_hand() ? 60 : 12 * 4;
    int tick_start = 0, tick_end = num_ticks - 1;

    if (settings.ShowWatchDialWindow) {
      // Only render parts under the window.  Alwaays start with a pos angle because div is weird with neg
      tick_start = ((window_start_angle + TRIG_MAX_ANGLE) * num_ticks + TRIG_MAX_ANGLE - 1) / TRIG_MAX_ANGLE;
      tick_end = ((window_end_angle + TRIG_MAX_ANGLE) * num_ticks) / TRIG_MAX_ANGLE;
    }

    for (int i = tick_start; i <= tick_end; i++) {
      // Angles are cyclical so it doesn't matter if i is in the range 0..59
      int angle_native = i * TRIG_MAX_ANGLE / num_ticks - TRIG_QUARTER_ANGLE;
      draw_minor_tick(ctx, angle_native, settings.MinorTickColor);
    }
  }

  if (settings.showMajorTick) {
    int hr_start = -1;
    int hr_end = 25;
    bool ends_reversed = 0;

    if (settings.ShowWatchDialWindow) {
      hr_start = modulus(((window_start_angle + TRIG_MAX_ANGLE) * 12 + TRIG_MAX_ANGLE - 1) / TRIG_MAX_ANGLE, 12);
      hr_end = modulus(((window_end_angle + TRIG_MAX_ANGLE) * 12) / TRIG_MAX_ANGLE, 12);
      ends_reversed = hr_start > hr_end;
    }

    for (int i = 0; i < 12; i++) {
      int angle_native = i * TRIG_MAX_ANGLE / 12 - TRIG_QUARTER_ANGLE;
      int tick_length = 16; // Length of the major tick
      GColor tick_color = (i == 6 || i == 12 || i == 3 || i == 9 || i == 0) ? settings.MajorTickColor : settings.MinorTickColor;

      if (settings.ShowWatchDialWindow &&
          ((!ends_reversed && (i < hr_start || i > hr_end)) ||
          (ends_reversed && (i < hr_start && i > hr_end)))) {
        tick_length = 8;  // major tick is smaller outside of the window
        tick_color = settings.MinimizedMajorTickColor;
      }

      draw_major_tick(ctx, angle_native, tick_length, tick_color, tick_color);
    }
  }
}


static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  memcpy(prv_tick_time, tick_time, sizeof(struct tm));
  current_date = prv_tick_time->tm_mday;
  s_weekday = prv_tick_time->tm_wday;
  seconds = prv_tick_time->tm_sec;
  #if defined(SUB_MINUTE_USE_TICK)
  sub_minute_interval = settings.SmoothMinuteHand ? prv_tick_time->tm_sec / settings.MinuteHandUpdateIntervalSec : 0;
  #endif
  minutes = prv_tick_time->tm_min;
  hours = prv_tick_time->tm_hour % 12;
  s_hours = prv_tick_time->tm_hour;

  #ifdef SHOW_MINUTE
  prv_tm.tm_min = minutes = SHOW_MINUTE;
  #endif

  hand_angle_native = calculate_hand_angle(prv_tick_time);

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  // Load fctx ffonts
  Date_Font = ffont_create_from_resource(RESOURCE_ID_FONT_DATE_FCTX);
  FontBTQTIconsFctx = ffont_create_from_resource(RESOURCE_ID_FONT_DRIPICONS_FCTX);

  // App exits when another app starts so this shouldn't change for this instance
  if (!alarm_service_peek_next(&next_alarm_time)) {
    next_alarm_time = 0;
  }

  // Subscribe to the connection service to get Bluetooth status updates.
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_vibe_icon
  });

#if defined(SUB_MINUTE_USE_APPTIMER)
  sub_minute_timer = app_timer_register_(1000 * settings.MinuteHandUpdateIntervalSec, apptimer_handler, NULL);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

#elif defined (SUB_MINUTE_USE_TICK)
  // Watchface is restarted when quiet time toggles so this is sufficient for QTBlankFace
  tick_timer_service_subscribe((use_minute_hand() && settings.SmoothMinuteHand) ?
    SECOND_UNIT : MINUTE_UNIT, tick_handler);
#endif

  //create layers
  s_bg_layer = layer_create(bounds);
  s_alarm_cal_pin_layer = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  s_date_battery_logo_layer = layer_create(bounds);

  // Change the order here
  layer_add_child(window_layer, s_bg_layer); //backforound, circles, major tick shoadow &tickmask
  layer_add_child(window_layer, s_alarm_cal_pin_layer);
  layer_add_child(window_layer, s_date_battery_logo_layer); //fctx version of text
  layer_add_child(window_layer, s_canvas_layer);  //hour and minute hands
 
  bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_alarm_cal_pin_layer, layer_update_proc_alarm_cal_pins);
  layer_set_update_proc(s_date_battery_logo_layer, update_logo_date_battery_fctx_layer);
  layer_set_update_proc(s_canvas_layer, hour_min_hands_canvas_update_proc);

  // Request an alarm/calendar re-sync on load if the stored data is stale.
  alarm_calendar_sync_maybe_request_update();
}


static void prv_window_unload(Window *window) {
  accel_tap_service_unsubscribe();
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();

#if defined(SUB_MINUTE_USE_APPTIMER)
  if (sub_minute_timer) {
    app_timer_cancel(sub_minute_timer);
    sub_minute_timer = NULL;
  }
#endif

  layer_destroy(s_canvas_layer);
  layer_destroy(s_alarm_cal_pin_layer);
  layer_destroy(s_bg_layer);
  layer_destroy(s_date_battery_logo_layer);
  ffont_destroy(Date_Font);
  ffont_destroy(FontBTQTIconsFctx);
}

static void prv_init(void) {
  prv_load_settings();

  // Open AppMessage and set the message handler. The alarm/calendar sync module
  // owns the inbox handler and forwards dictionaries to the settings handler.
  alarm_calendar_sync_init(prv_inbox_received_handler);
  alarm_calendar_sync_set_enabled(settings.EnableAlarmCalendarSync);
  alarm_calendar_sync_set_alarm_pin(settings.TimelineAlarmPin);
  alarm_calendar_sync_set_timer_pin(settings.TimelineTimerPin);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  window_stack_push(s_window, true);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

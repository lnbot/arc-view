#include <pebble.h>
#include "Monologue.h"
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
static Layer *s_dial_layer;
//static Layer *s_dial_digits_layer;
static Layer *s_date_battery_logo_layer;
//static Layer *s_canvas_second_hand;
static Layer *s_canvas_bt_icon;
static Layer *s_canvas_qt_icon;
static Layer *s_canvas_battery;
static GRect bounds;
//static GRect bounds_seconds;
// Fonts
static GFont
    #ifdef PBL_BW
    FontDate,
    FontBattery,
    FontLogo,
    FontHour,
    #endif
    FontBTQTIcons;

// Smooth interval for updating the minute hand
const int SMOOTH_WAKEUP_COOKIE = 1;

FFont* Date_Font;
// Time and date variables
static struct tm *prv_tick_time;
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
  int BottomXPosition;
  int DateBottomYPosition;
  int BTQTBottomYPosition;
  int LeftxPosition;
  int xOffset;
  int yOffset;
  int xOffsetFctxWeekday;
  int xOffsetFctxDate;
  int yOffsetFctx;
  int xyMaskOffset;
  int xWeekdayOffset;
  int yWeekdayDateOffset;
  int xDateOffset;
  int ShadowAndMaskWidth;
  int ShadowAndMaskHeight;
  int WeekdayWidth;
  int DateWidth;
  int WeekdayDateHeight;
  int BTIconYOffset;
  int QTIconYOffset;
  int BatteryYOffset;
  int BatteryYOffset2;
  int BatteryYOffset3;
  int BatteryLineYOffset;
  int BatteryLineYOffset2;
  int LogoYOffset;
  int LogoYOffset2;
  int LogoYOffset3;
  int font_size_digits;
  int font_size_battery;
  int font_size_date;
  int font_size_logo;
  int six_pos_x;
  int six_pos_y;
  int twelve_pos_x;
  int twelve_pos_y;
  int date_pos_x;
  int date_pos_y;
  int battery_pos_z;
  int battery_pos_y;
  int battery_line;
  int analogue_hand_a;
  int analogue_hand_b;
  int analogue_hand_c;
  int hour_hand_a;
  int min_hand_a;
  int circle_radius_adj;
  int tick_mask_radius_adj;
  int hands_shadow;
  int QTIconXOffset2;
  int BTIconXOffset2;
  int QTIconYOffset2;
  int BTIconYOffset2;
  int corner_radius_secondshand;
  int corner_radius_majortickrect;
  int corner_radius_minortickrect;
  int majortickrect_w;
  int majortickrect_h;
  int minortickrect_w;
  int minortickrect_h;
  int tick_inset_outer;
  int SecondsCentreOuterRadius;
  int SecondsCentreInnerRadius;
  int ComplicationBorderAdj;
  int ComplicationDistanceAdj;
  int ComplicationOrbitSizeAdj;
  GRect dial_digits_mask_a[1];
  GRect dial_digits_mask_b[1];
  GRect dial_digits_mask_c[1];
} UIConfig;

#ifdef PBL_PLATFORM_GABBRO
#define RESOURCE_ID_FONT_DRIPICONS_X RESOURCE_ID_FONT_DRIPICONS_18
#else
#define RESOURCE_ID_FONT_DRIPICONS_X RESOURCE_ID_FONT_DRIPICONS_16
#endif

#ifdef PBL_PLATFORM_EMERY
static const UIConfig config = {
.BottomXPosition = 46,
.DateBottomYPosition = 168,
.BTQTBottomYPosition = 168,
.LeftxPosition = -5,
.xOffset = 22,
.yOffset = -8,
.xOffsetFctxWeekday = 38/2  -1,
.xOffsetFctxDate = 22/2 + 1,
.yOffsetFctx = 3,
.xyMaskOffset = 6,
.xWeekdayOffset = 2,
.yWeekdayDateOffset = 1,
.xDateOffset = 41,
.ShadowAndMaskWidth = 64,
.ShadowAndMaskHeight = 17,
.WeekdayWidth = 38,
.DateWidth = 22,
.WeekdayDateHeight = 13,
.BTIconYOffset = -21,
.QTIconYOffset = 21,
.BatteryYOffset = 53-4 +3-1,
.BatteryYOffset2 = -9,
.BatteryYOffset3 = -9,
.BatteryLineYOffset = 66,
.BatteryLineYOffset2 = -8+114,
.LogoYOffset = 69+1,
.LogoYOffset2 = -8,
.LogoYOffset3 = -8+114,
.font_size_digits = 36,
.font_size_battery = 14,
.font_size_date = 12,
.font_size_logo = 10,
.six_pos_x = 2,
.six_pos_y = -40 + 9,
.twelve_pos_x = - 2 + 1,
.twelve_pos_y = -3,
.battery_pos_z = -2,
.battery_pos_y = 4,
.battery_line = 63, //sized to the width of the default logo TITANIUM
.analogue_hand_a = 1,  //was 20
.analogue_hand_b = 28,
.analogue_hand_c = 20,
.hour_hand_a = 45,
.min_hand_a = 2,  //was 20
.circle_radius_adj = 18,
.tick_mask_radius_adj = 12,
.hands_shadow = 2,
.QTIconXOffset2 =0,
.QTIconYOffset2 = 0,
.BTIconXOffset2 = 0,
.BTIconYOffset2 = 0,
.corner_radius_secondshand = 20,
.corner_radius_majortickrect = 20,
.corner_radius_minortickrect = 20,
.majortickrect_w = 86,
.majortickrect_h = 100,
.minortickrect_w = 90,
.minortickrect_h = 104,
.tick_inset_outer = -10,
.SecondsCentreOuterRadius = 9,
.SecondsCentreInnerRadius = 2,
.dial_digits_mask_a = {{{100-15,23},{39,7}}},
.dial_digits_mask_b = {{{100-19,0},{39,27}}},
.dial_digits_mask_c = {{{100-15,228-27},{31,27}}},
.ComplicationBorderAdj = 4,
.ComplicationDistanceAdj = 6,
.ComplicationOrbitSizeAdj = 2,
};
#elif defined(PBL_PLATFORM_GABBRO)
static const UIConfig config = {
.BottomXPosition = 46+30,
.DateBottomYPosition = 178,
.BTQTBottomYPosition = 178,
.LeftxPosition = 16+2,
.xOffset = 22,
.yOffset = -8,
.xOffsetFctxWeekday = 38/2  -1,
.xOffsetFctxDate = 22/2 + 1,
.yOffsetFctx = 3,
.xyMaskOffset = 6,
.xWeekdayOffset = 2,
.yWeekdayDateOffset = 1,
.xDateOffset = 41,
.ShadowAndMaskWidth = 64,
.ShadowAndMaskHeight = 17,
.WeekdayWidth = 38,
.DateWidth = 22,
.WeekdayDateHeight = 13,
.BTIconYOffset = -21,
.QTIconYOffset = 21,
.BatteryYOffset = 53-4 +3-1,
.BatteryYOffset2 = -9,
.BatteryYOffset3 = -1,
.BatteryLineYOffset = 66,
.BatteryLineYOffset2 = -8+130,
.LogoYOffset = 69+1+1,
.LogoYOffset2 = -8,
.LogoYOffset3 = -8+130,
.font_size_digits = 44,
.font_size_battery = 14,
.font_size_date = 12,
.font_size_logo = 10,
.six_pos_x = 2,
.six_pos_y = - 40 + 9,
.twelve_pos_x = - 2 + 1,
.twelve_pos_y = -3,
.battery_pos_z = -2,
.battery_pos_y = 4,
.battery_line = 63,
.analogue_hand_a = 3+8,
.analogue_hand_b = 32,
.hour_hand_a = 60,
.min_hand_a = 2,
.circle_radius_adj = 17,
.tick_mask_radius_adj = 12,
.hands_shadow = 2,
.QTIconXOffset2 =0,
.QTIconYOffset2 = 0,
.BTIconXOffset2 = 0,
.BTIconYOffset2 = 0,
.analogue_hand_c = 40,
.SecondsCentreOuterRadius = 11,
.SecondsCentreInnerRadius = 3,
.dial_digits_mask_a = {{{130-15,23-2},{39,7+2}}},
.dial_digits_mask_b = {{{130-19,0},{39,27}}},
.dial_digits_mask_c = {{{130-15,260-27},{31,27}}},
.ComplicationBorderAdj = 2,
.ComplicationDistanceAdj = 2,
.ComplicationOrbitSizeAdj = 3,
};
#elif defined(PBL_BW)
static const UIConfig config = {
.BottomXPosition = 38,
.DateBottomYPosition = 126-3+5,
.BTQTBottomYPosition = 126-3,
.LeftxPosition = 6 - 2,
.xOffset = 10,
.yOffset = -7,
.xOffsetFctxWeekday = 30/2 ,
.xOffsetFctxDate = 16/2 +1,
.yOffsetFctx = 2,
.xyMaskOffset = 3,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 16,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18,
.BatteryYOffset = 40-4,
.BatteryYOffset2 = -5,
.BatteryYOffset3 = -5,
.BatteryLineYOffset = 49,
.BatteryLineYOffset2 = -8+84,
.LogoYOffset = 52+1,
.LogoYOffset2 = -5,
.LogoYOffset3 = -5+84,
.font_size_digits = 24,
.font_size_battery = 10,
.font_size_date = 9,
.font_size_logo = 8,
.six_pos_x = 2,
.six_pos_y = - 38 + 9,
.twelve_pos_x = - 2 + 1,
.twelve_pos_y = -2,
.battery_pos_z = 0,
.battery_pos_y = 4,
.battery_line = 51,
.analogue_hand_a = 1,
.analogue_hand_b = 22,
.analogue_hand_c = 20,
.hour_hand_a = 35,
.min_hand_a = 22,
.circle_radius_adj = 18,
.tick_mask_radius_adj = 12,
.hands_shadow = 2,
.QTIconXOffset2 = 42,
.QTIconYOffset2 = 23,
.BTIconXOffset2 = -29,
.BTIconYOffset2 = 23,
.corner_radius_secondshand = 15,
.corner_radius_majortickrect = 15,
.corner_radius_minortickrect = 15,
.majortickrect_w = 62,
.majortickrect_h = 72,
.minortickrect_w = 66,
.minortickrect_h = 76,
.tick_inset_outer = -10,
.SecondsCentreOuterRadius = 7,
.SecondsCentreInnerRadius = 1,
.dial_digits_mask_a = {{{72-14,22},{36,7}}},
.dial_digits_mask_b = {{{72-18,0},{36,26}}},
.dial_digits_mask_c = {{{72-13,168-26},{28,26}}},
.ComplicationBorderAdj = 0,
.ComplicationDistanceAdj = 0,
.ComplicationOrbitSizeAdj = 0,
};
#elif defined(PBL_ROUND)
static const UIConfig config = {
.BottomXPosition = 52,
.DateBottomYPosition = 132,
.BTQTBottomYPosition = 132,
.LeftxPosition = 16,
.xOffset = 14,
.yOffset = -7,
.xOffsetFctxWeekday = 30/2 -1,
.xOffsetFctxDate = 16/2 +1,
.yOffsetFctx = 2,
.xyMaskOffset = 4,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 16,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18,
.BatteryYOffset = 40-4,
.BatteryYOffset2 = -4,
.BatteryYOffset3 = -4,
.BatteryLineYOffset = 49,
.BatteryLineYOffset2 = -8+90+4,
.LogoYOffset = 52+1,
.LogoYOffset2 = -8,
.LogoYOffset3 = -8+90+4,
.font_size_digits = 28,
.font_size_battery = 10,
.font_size_date = 9,
.font_size_logo = 8,
.six_pos_x = 2,
.six_pos_y = - 38 + 9,
.twelve_pos_x = - 2 + 1,
.twelve_pos_y = -2,
.battery_pos_z = 0,
.battery_pos_y = 4,
.battery_line = 51,
.analogue_hand_a = 3+8,
.analogue_hand_b = 22,
.hour_hand_a = 50,
.min_hand_a = 34,
.circle_radius_adj = 17,
.tick_mask_radius_adj = 12,
.hands_shadow = 2,
.QTIconXOffset2 =0,
.QTIconYOffset2 = 0,
.BTIconXOffset2 = 0,
.BTIconYOffset2 = 0,
.analogue_hand_c = 28,
.SecondsCentreOuterRadius = 7,
.SecondsCentreInnerRadius = 1,
.dial_digits_mask_a = {{{90-14,22},{36,7}}},
.dial_digits_mask_b = {{{90-18,0},{36,26}}},
.dial_digits_mask_c = {{{90-13,180-26},{28,26}}},
.ComplicationBorderAdj = -1,
.ComplicationDistanceAdj = -1,
.ComplicationOrbitSizeAdj = 2,
};
#else // Default for other platforms
static const UIConfig config = {
.BottomXPosition = 38,
.DateBottomYPosition = 126-3+5,
.BTQTBottomYPosition = 126-3,
.LeftxPosition = 7 - 2,
.xOffset = 10,
.yOffset = -7,
.xOffsetFctxWeekday = 30/2 -1,
.xOffsetFctxDate = 16/2 +1,
.yOffsetFctx = 2,
.xyMaskOffset = 4,
.xWeekdayOffset = 1,
.yWeekdayDateOffset = 2,
.xDateOffset = 31,
.ShadowAndMaskWidth = 48,
.ShadowAndMaskHeight = 15,
.WeekdayWidth = 30,
.DateWidth = 16,
.WeekdayDateHeight = 11,
.BTIconYOffset = -20,
.QTIconYOffset = 18,
.BatteryYOffset = 40-4,
.BatteryYOffset2 = -7,
.BatteryYOffset3 = -7,
.BatteryLineYOffset = 49,
.BatteryLineYOffset2 = -8+84+1,
.LogoYOffset = 52+1,
.LogoYOffset2 = -8,
.LogoYOffset3 = -8+84,
.font_size_digits = 24,
.font_size_battery = 10,
.font_size_date = 9,
.font_size_logo = 8,
.six_pos_x = 2,
.six_pos_y = - 38 + 9,
.twelve_pos_x = - 2 + 1,
.twelve_pos_y = -2,
.battery_pos_z = 0,
.battery_pos_y = 4,
.battery_line = 51,
.analogue_hand_a = 1,
.analogue_hand_b = 22,
.analogue_hand_c = 3,
.hour_hand_a = 35,
.min_hand_a = 22,
.circle_radius_adj = 18,
.tick_mask_radius_adj = 12,
.hands_shadow = 2,
.QTIconXOffset2 = 42,
.QTIconYOffset2 = 23,
.BTIconXOffset2 = -29,
.BTIconYOffset2 = 23,
.corner_radius_secondshand = 15,
.corner_radius_majortickrect = 15,
.corner_radius_minortickrect = 15,
.majortickrect_w = 62,
.majortickrect_h = 72,
.minortickrect_w = 66,
.minortickrect_h = 76,
.tick_inset_outer = -10,
.SecondsCentreOuterRadius = 7,
.SecondsCentreInnerRadius = 1,
.dial_digits_mask_a = {{{72-14,22},{36,7}}},
.dial_digits_mask_b = {{{72-18,0},{36,26}}},
.dial_digits_mask_c = {{{72-13,168-26},{28,26}}},
.ComplicationBorderAdj = 2,
.ComplicationDistanceAdj = 2
};
#endif

bool connected = true;

//function prototypes

static void prv_save_settings(void);
static void prv_default_settings(void);
static void prv_load_settings(void);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
#if defined(SUB_MINUTE_USE_APPTIMER)
static void apptimer_handler(void *data);
#endif
static void bg_update_proc(Layer *layer, GContext *ctx);
//static void layer_update_proc_dial_digits_mask(Layer *layer, GContext * ctx);
static void update_logo_date_battery_fctx_layer(Layer *layer, GContext * ctx);
static void layer_update_proc_battery_line(Layer *layer, GContext * ctx);
//static void layer_update_proc_seconds_hand(Layer *layer, GContext * ctx);
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_qt(Layer *layer, GContext *ctx);
static void layer_update_proc_bt(Layer *layer, GContext *ctx);
// static void draw_fancy_hand_hour(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
// static void draw_fancy_hand_min(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
static int calculate_hand_angle(struct tm *tick_time);
static void draw_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color);
//static void draw_center(GContext *ctx, GColor minutes_border, GColor minutes_color);
static void draw_hand_center(GContext *ctx, GColor outer_color, GColor inner_color);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);

// Save settings to persistent storage
static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}


// Set default settings
static void prv_default_settings(void) {
  // settings.EnableSecondsHand = true;
  // settings.SecondsVisibleTime = 135;
  settings.EnableDate = true;
  settings.EnableBattery = true;
  settings.EnableBatteryLine = true;
  settings.EnableLogo = true;
  snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "pebble");
  settings.BackgroundColor1 = GColorWhite;
  settings.ComplicationBorderColor = GColorLightGray;
  settings.ComplicationShadowColor = GColorWhite;
  settings.MinuteHandShadowColor = GColorBabyBlueEyes;
  settings.TextColor1 = GColorWhite;
  settings.MinorTickColor = GColorBabyBlueEyes;
  settings.TextColor3 = GColorDarkGray;
  settings.DateColor = GColorDarkGray;
  settings.HourDigitsColor = GColorCobaltBlue;
  settings.HoursHandBorderColor = GColorDarkGray;
  settings.MinutesHandColor = GColorCobaltBlue;
  settings.MinutesHandBorderColor = GColorCobaltBlue;
  settings.MajorTickColor = GColorCobaltBlue;
  settings.SecondsHandColor = GColorOrange;
  settings.BatteryLineColor = GColorOrange;
  settings.BWDateColor = GColorBlack;
  settings.BWBackgroundColor1 = GColorWhite;
  settings.BWMinuteHandShadowColor = GColorDarkGray;
  settings.BWMinHandBatLineColor = GColorBlack;
  settings.BWHourDigitsColor = GColorBlack;
  settings.BWMajorTickColor = GColorBlack;
  settings.BTQTColor = GColorDarkGray;
  settings.BWBTQTColor = GColorBlack;
  settings.showMajorTick = true;
  settings.showMinorTick = true;
  // settings.BWThemeSelect = "wh";
  // settings.ThemeSelect = "wh";
  snprintf(settings.BWThemeSelect, sizeof(settings.BWThemeSelect), "%s", "wh");
  snprintf(settings.ThemeSelect, sizeof(settings.ThemeSelect), "%s", "wh");
  snprintf(settings.PosLeft, sizeof(settings.PosLeft), "%s", "hr");
  snprintf(settings.PosRight, sizeof(settings.PosRight), "%s", "lo");
  snprintf(settings.PosTop, sizeof(settings.PosTop), "%s", "ap");
  snprintf(settings.PosBottom, sizeof(settings.PosBottom), "%s", "dt");
  settings.BWShadowOn = true;
  settings.ShadowOn = true;
  settings.Font = 1;
  settings.VibeOn = false;
  settings.AddZero12h = false;
  settings.RemoveZero24h = false;
  //settings.showlocalAMPM = true;
  settings.ForegroundShape = true;  //true = round, false = rect
  settings.CentreSize = config.SecondsCentreOuterRadius;
  settings.InnerCentreSize = config.SecondsCentreInnerRadius;
  settings.HandThickness = 2;
  settings.DigitalHour = true;
  settings.BackSize = 4;
  settings.BackLen = config.analogue_hand_b;
  settings.ComplicationFontSizeAdj = 0;
  settings.SmoothMinuteHand = false;
  settings.MinuteHandUpdateIntervalSec = 60;
  settings.OrbitComplications = false;
}

// Quiet time icon handler
static void quiet_time_icon () {
    layer_set_hidden(s_canvas_qt_icon, !quiet_time_is_active());
}


static void bluetooth_vibe_icon (bool connected) {

   layer_set_hidden(s_canvas_bt_icon, connected);

  if((!connected && !quiet_time_is_active()) ||(!connected && quiet_time_is_active() && settings.VibeOn)) {
    // Issue a vibrating alert
    #ifdef PBL_PLATFORM_DIORITE
    vibes_short_pulse();

    #else
    vibes_double_pulse();
    #endif

}
}

// Load settings from persistent storage
static void prv_load_settings(void) {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// AppMessage inbox handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Received message");
#endif

  bool settings_changed = false;
  bool theme_settings_changed = false;

  Tuple *vibe_t = dict_find(iter, MESSAGE_KEY_VibeOn);
  Tuple *enable_date_t = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_battery_t = dict_find(iter, MESSAGE_KEY_EnableBattery);
  Tuple *enable_battery_line_t = dict_find(iter, MESSAGE_KEY_EnableBatteryLine);
  Tuple *enable_logo_t = dict_find(iter, MESSAGE_KEY_EnableLogo);
  Tuple *logotext_t = dict_find(iter, MESSAGE_KEY_LogoText);
  Tuple *bwthemeselect_t = dict_find(iter, MESSAGE_KEY_BWThemeSelect);
  Tuple *themeselect_t = dict_find(iter, MESSAGE_KEY_ThemeSelect);
  Tuple *bg_color1_t = dict_find(iter, MESSAGE_KEY_BackgroundColor1);
  Tuple *comp_border_color_t = dict_find(iter, MESSAGE_KEY_ComplicationBorderColor);
  Tuple *comp_shadow_color_t = dict_find(iter, MESSAGE_KEY_ComplicationShadowColor);
  Tuple *bg_color2_t = dict_find(iter, MESSAGE_KEY_MinuteHandShadowColor);
  //Tuple *text_color1_t = dict_find(iter, MESSAGE_KEY_TextColor1);
  Tuple *text_color2_t = dict_find(iter, MESSAGE_KEY_MinorTickColor);
  //Tuple *text_color3_t = dict_find(iter, MESSAGE_KEY_TextColor3);
  Tuple *date_color_t = dict_find(iter, MESSAGE_KEY_DateColor);
  Tuple *bwdate_color_t = dict_find(iter, MESSAGE_KEY_BWDateColor);
  Tuple *hours_color_t = dict_find(iter, MESSAGE_KEY_HourDigitsColor);
  //Tuple *hours_border_t = dict_find(iter, MESSAGE_KEY_HoursHandBorderColor);
  Tuple *minutes_color_t = dict_find(iter, MESSAGE_KEY_MinutesHandColor);
  //Tuple *minutes_border_t = dict_find(iter, MESSAGE_KEY_MinutesHandBorderColor);
  Tuple *tick_color_t = dict_find(iter, MESSAGE_KEY_MajorTickColor);
  //Tuple *seconds_color_t = dict_find(iter, MESSAGE_KEY_SecondsHandColor);
  Tuple *battery_line_color_t = dict_find(iter, MESSAGE_KEY_BatteryLineColor);
  Tuple *bwbg_color1_t = dict_find(iter, MESSAGE_KEY_BWBackgroundColor1);
  Tuple *bwbg_color2_t = dict_find(iter, MESSAGE_KEY_BWMinuteHandShadowColor);
  Tuple *bwtext_color1_t = dict_find(iter, MESSAGE_KEY_BWMinHandBatLineColor);
  Tuple *bwtext_color2_t = dict_find(iter, MESSAGE_KEY_BWHourDigitsColor);
  Tuple *bwtext_color3_t = dict_find(iter, MESSAGE_KEY_BWMajorTickColor);
  Tuple *btqt_color_t = dict_find(iter, MESSAGE_KEY_BTQTColor);
  Tuple *bwbtqt_color_t = dict_find(iter, MESSAGE_KEY_BWBTQTColor);
  Tuple *bwshadowon_t = dict_find(iter, MESSAGE_KEY_BWShadowOn);
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

  if (fg_shape_t) {
    settings.ForegroundShape = fg_shape_t->value->int32 == 1;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
  }

  if (dig_t) {
    settings.DigitalHour = dig_t->value->int32 != 0;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (ocent_t) {
    settings.CentreSize = (int) ocent_t -> value -> int32;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (icent_t) {
    settings.InnerCentreSize = (int) icent_t -> value -> int32;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (hand_t) {
    settings.HandThickness = (int) hand_t -> value -> int32;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (back_t) {
    settings.BackSize = (int) back_t -> value -> int32;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (backlen_t) {
    settings.BackLen = (int) backlen_t -> value -> int32;
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if(majort_t){
    settings.showMajorTick = majort_t->value->int32 != 0;
    layer_mark_dirty(s_bg_layer);
  } 
  
  if(minort_t){
    settings.showMinorTick = minort_t->value->int32 != 0;
    layer_mark_dirty(s_bg_layer);
  }

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

  if (vibe_t){
    if (vibe_t -> value -> int32 == 0){
      settings.VibeOn = false;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe off");
    } else {
      settings.VibeOn = true;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe on");
    }
    layer_mark_dirty(s_canvas_bt_icon);
  }

  if (addzero12_t) {
    settings.AddZero12h = addzero12_t->value->int32 != 0;
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (remzero24_t) {
    settings.RemoveZero24h = remzero24_t->value->int32 != 0;
     layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (enable_logo_t) {
    settings.EnableLogo = enable_logo_t->value->int32 == 1;

    // Check if the logo is enabled and the custom text string is not empty
    if (settings.EnableLogo && logotext_t && strlen(logotext_t->value->cstring) > 0) {
      // If the custom text field is not blank, use the user's text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", logotext_t->value->cstring);
    } else if (settings.EnableLogo && strlen(logotext_t->value->cstring) == 0) {
      // If the custom text field is blank but the logo is enabled, use the default text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "monologue");
    }
    else {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "");
    }

    layer_mark_dirty(s_date_battery_logo_layer);

  }

  if (enable_battery_t) {
    settings.EnableBattery = enable_battery_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_battery);
    layer_mark_dirty(s_date_battery_logo_layer);
    }

  if (enable_battery_line_t) {
    settings.EnableBatteryLine = enable_battery_line_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_battery);
  }

  if (complication_font_size_adj_t) {
    settings.ComplicationFontSizeAdj = complication_font_size_adj_t->value->int32;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_battery);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

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
    tick_timer_service_subscribe(settings.SmoothMinuteHand ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
#endif

    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (orbit_complications_t) {
    settings.OrbitComplications = orbit_complications_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_battery);
    layer_mark_dirty(s_date_battery_logo_layer);
  }

  if (bwthemeselect_t) {
          // Compare the string value received from the phone
          if (strcmp(bwthemeselect_t->value->cstring, "wh") == 0) {
              // Set the theme and other settings for "wh"
                    settings.BWDateColor = GColorBlack;
                    if (bwshadowon_t) {
                      settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
                    }
                        if(settings.BWShadowOn){
                          settings.BWMinuteHandShadowColor = GColorDarkGray;
                        }
                        else {
                        settings.BWMinuteHandShadowColor = GColorWhite;
                        }
                    settings.BWBackgroundColor1 = GColorWhite;
                    settings.BWMinHandBatLineColor = GColorBlack;
                    settings.BWHourDigitsColor = GColorBlack;
                    settings.BWMajorTickColor = GColorBlack;
                    settings.BWBTQTColor = GColorBlack;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme white selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "bl") == 0) {
              // Set the theme and other settings for "bl"
                    settings.BWDateColor = GColorWhite;
                    settings.BWBackgroundColor1 = GColorBlack;
                    if (bwshadowon_t) {
                      settings.BWShadowOn = bwshadowon_t->value->int32 == 1;
                    }
                        if(settings.BWShadowOn){
                          settings.BWMinuteHandShadowColor = GColorDarkGray;
                        }
                        else {
                        settings.BWMinuteHandShadowColor = GColorBlack;
                        }
                    settings.BWMinHandBatLineColor = GColorWhite;
                    settings.BWHourDigitsColor = GColorWhite;
                    settings.BWMajorTickColor = GColorWhite;
                    settings.BWBTQTColor = GColorWhite;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "cu") == 0) {
              // Set the theme for "cu" and handle custom colors
              settings.BWDateColor = GColorFromHEX(bwdate_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);

                  if (bwbg_color1_t) {
                    settings.BWBackgroundColor1 = GColorFromHEX(bwbg_color1_t->value->int32);
                    settings_changed = true;
                  }

                  if (bwshadowon_t) {
                    settings.BWShadowOn = bwshadowon_t->value->int32 == 1;

                      if(settings.BWShadowOn){
                        if (bwbg_color2_t) {
                          settings.BWMinuteHandShadowColor = GColorFromHEX(bwbg_color2_t->value->int32);
                          settings_changed = true;
                        }
                      }
                      else {
                      settings.BWMinuteHandShadowColor = settings.BWBackgroundColor1;
                      }
                  }

                  if (bwtext_color1_t) {
                    settings.BWMinHandBatLineColor = GColorFromHEX(bwtext_color1_t->value->int32);
                    settings_changed = true;
                  }
                  if (bwtext_color2_t) {
                    settings.BWHourDigitsColor = GColorFromHEX(bwtext_color2_t->value->int32);
                    settings_changed = true;
                  }
                  if (bwtext_color3_t) {
                    settings.BWMajorTickColor = GColorFromHEX(bwtext_color3_t->value->int32);
                    settings_changed = true;
                  }
                  if (bwbtqt_color_t) {
                    settings.BWBTQTColor = GColorFromHEX(bwbtqt_color_t->value->int32);
                    layer_mark_dirty(s_canvas_bt_icon);
                    layer_mark_dirty(s_canvas_qt_icon);
                  }
                  theme_settings_changed = true;
                  //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
                }
          }
/////////////////////////////////////
  if (themeselect_t) {
          // Compare the string value received from the phone
          if (strcmp(themeselect_t->value->cstring, "wh") == 0) {
              // Set the theme and other settings for "wh"
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.MinuteHandShadowColor = GColorBabyBlueEyes;
                        }
                        else {
                        settings.MinuteHandShadowColor = GColorWhite;
                        }
                    settings.BackgroundColor1 = GColorWhite;
                    settings.ComplicationBorderColor = GColorLightGray;
                    settings.ComplicationShadowColor = GColorLightGray;
                    settings.MinorTickColor = GColorBabyBlueEyes;
                    settings.DateColor = GColorDarkGray;
                    settings.HourDigitsColor = GColorCobaltBlue;
                    settings.MinutesHandColor = GColorCobaltBlue;
                    settings.MajorTickColor = GColorCobaltBlue;
                    settings.BatteryLineColor = GColorOrange;
                    settings.BTQTColor = GColorDarkGray;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme white selected");
          } else if (strcmp(themeselect_t->value->cstring, "bl") == 0) {
              // Set the theme and other settings for "bl"

                    settings.BackgroundColor1 = GColorBlack;
                    settings.ComplicationBorderColor = GColorBlack;
                    settings.ComplicationShadowColor = GColorBlack;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.MinuteHandShadowColor = GColorDarkGray;
                        }
                        else {
                        settings.MinuteHandShadowColor = GColorBlack;
                        }
                    settings.MinorTickColor = GColorDarkGray;
                    settings.DateColor = GColorWindsorTan;
                    settings.HourDigitsColor = GColorYellow;
                    settings.MinutesHandColor = GColorYellow;
                    settings.MajorTickColor = GColorYellow;
                    settings.BatteryLineColor = GColorYellow;
                    settings.BTQTColor = GColorLightGray;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
          } else if (strcmp(themeselect_t->value->cstring, "bu") == 0) {
              // Set the theme and other settings for "bl"

                    settings.BackgroundColor1 = GColorDukeBlue;
                    settings.ComplicationBorderColor = GColorDukeBlue;
                    settings.ComplicationShadowColor = GColorDukeBlue;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.MinuteHandShadowColor = GColorOxfordBlue;
                        }
                        else {
                        settings.MinuteHandShadowColor = GColorDukeBlue;
                        }
                    settings.MinorTickColor = GColorPictonBlue;
                    settings.DateColor = GColorWhite;
                    settings.HourDigitsColor = GColorYellow;
                    settings.MinutesHandColor = GColorYellow;
                    settings.MajorTickColor = GColorYellow;
                    settings.BatteryLineColor = GColorRed;
                    settings.BTQTColor = GColorPictonBlue;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme blue selected");
          } else if (strcmp(themeselect_t->value->cstring, "pl") == 0) {
              // Set the theme and other settings for "bl"

                    settings.BackgroundColor1 = GColorPurple;
                    settings.ComplicationBorderColor = GColorPurple;
                    settings.ComplicationShadowColor = GColorPurple;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.MinuteHandShadowColor = GColorImperialPurple;
                        }
                        else {
                        settings.MinuteHandShadowColor = GColorPurple;
                        }
                    settings.MinorTickColor = GColorImperialPurple;
                    settings.DateColor = GColorRichBrilliantLavender;
                    settings.HourDigitsColor = GColorRichBrilliantLavender;
                    settings.MinutesHandColor = GColorRichBrilliantLavender;
                    settings.MajorTickColor = GColorRichBrilliantLavender;
                    settings.BatteryLineColor = GColorBulgarianRose;
                    settings.BTQTColor = GColorImperialPurple;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme purple selected");
          } else if (strcmp(themeselect_t->value->cstring, "gr") == 0) {
              // Set the theme and other settings for "gr"

                    settings.BackgroundColor1 = GColorBlack;
                    settings.ComplicationBorderColor = GColorBlack;
                    settings.ComplicationShadowColor = GColorBlack;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.MinuteHandShadowColor = GColorDarkGreen;
                        }
                        else {
                        settings.MinuteHandShadowColor = GColorBlack;
                        }
                    settings.MinorTickColor = GColorDarkGreen;
                    settings.DateColor = GColorGreen;
                    settings.HourDigitsColor = GColorBrightGreen;
                    settings.MinutesHandColor = GColorBrightGreen;
                    settings.MajorTickColor = GColorBrightGreen;
                    settings.BatteryLineColor = GColorPastelYellow;
                    settings.BTQTColor = GColorDarkGreen;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black & green selected");
          } else if (strcmp(themeselect_t->value->cstring, "cu") == 0) {
              // Set the theme for "cu" and handle custom colors
                  if (bg_color1_t) {
                    settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32);
                    settings_changed = true;
                  }

                  if (comp_border_color_t) {
                    settings.ComplicationBorderColor = GColorFromHEX(comp_border_color_t->value->int32);
                    settings_changed = true;
                  }

                  if (comp_shadow_color_t) {
                    settings.ComplicationShadowColor = GColorFromHEX(comp_shadow_color_t->value->int32);
                    settings_changed = true;
                  }

                  if (shadowon_t) {
                    settings.ShadowOn = shadowon_t->value->int32 == 1;

                      if(settings.ShadowOn){
                        if (bg_color2_t) {
                          settings.MinuteHandShadowColor = GColorFromHEX(bg_color2_t->value->int32);
                          settings_changed = true;
                        }
                      }
                      else {
                      settings.MinuteHandShadowColor = settings.BackgroundColor1;
                      }
                  }

                  if (text_color2_t) {
                    settings.MinorTickColor = GColorFromHEX(text_color2_t->value->int32);
                    layer_mark_dirty(s_bg_layer);
                  }
                  
                  if (date_color_t) {
                    settings.DateColor = GColorFromHEX(date_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                    layer_mark_dirty(s_date_battery_logo_layer);
                  }
                  if (hours_color_t) {
                    settings.HourDigitsColor = GColorFromHEX(hours_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                   // layer_mark_dirty(s_canvas_second_hand);
                  }
                 
                  if (minutes_color_t) {
                    settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                  //  layer_mark_dirty(s_canvas_second_hand);
                    layer_mark_dirty(s_date_battery_logo_layer);
                  }
                 
                  if (tick_color_t) {
                    settings.MajorTickColor = GColorFromHEX(tick_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                    layer_mark_dirty(s_date_battery_logo_layer);
                  }
                  
                  if (battery_line_color_t) {
                    settings.BatteryLineColor = GColorFromHEX(battery_line_color_t->value->int32);
                    layer_mark_dirty(s_canvas_battery);
                  }
                  if (btqt_color_t) {
                    settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32);
                    layer_mark_dirty(s_canvas_bt_icon);
                    layer_mark_dirty(s_canvas_qt_icon);
                  }
                  theme_settings_changed = true;
                //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
                }
          }

                  ///////////////////////////////

  if (settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
   // layer_mark_dirty(s_dial_digits_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  //  layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_canvas_battery);
  }

  if (theme_settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_dial_layer);
  //  layer_mark_dirty(s_dial_digits_layer);
    layer_mark_dirty(s_date_battery_logo_layer);
  //  layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_canvas_qt_icon);
    layer_mark_dirty(s_canvas_bt_icon);
    layer_mark_dirty(s_canvas_battery);
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
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler fired: %02d:%02d:%02d", tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);

  time_t curr_time = time(NULL);
  prv_tick_time = localtime(&curr_time);
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
    hand_angle_native = calculate_hand_angle(prv_tick_time);

    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_date_battery_logo_layer);

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
  GColor shadow_color = PBL_IF_BW_ELSE(settings.BWMinuteHandShadowColor,settings.MinuteHandShadowColor);


  // Set the antialiasing
  graphics_context_set_antialiased(ctx, true);

  // Draw the shadow first, with a small offset
  graphics_context_set_stroke_color(ctx, shadow_color);
  graphics_context_set_fill_color(ctx, shadow_color);
  graphics_context_set_stroke_width(ctx, settings.HandThickness); // Same width as the hand
  
  #ifdef PBL_COLOR
    graphics_draw_line(ctx, 
        GPoint(p3.x, p3.y), 
        GPoint(p4.x, p4.y)
      );
  #else  //switch to a fill as grey is not available as a line colour on BW screens
        GPoint s1 = GPoint(p3.x, p3.y);
        GPoint s2 = GPoint(p4.x, p4.y);

        int sdx = s2.x - s1.x;
        int sdy = s2.y - s1.y;
        int len_sq = sdx*sdx + sdy*sdy;
        int len = 1;
        while (len * len < len_sq) len++;

        int px = (sdy * settings.HandThickness ) / len;
        int py = (sdx * settings.HandThickness ) / len;

        GPoint shadow_points[4] = {
          GPoint(s1.x - px, s1.y + py),
          GPoint(s1.x + px, s1.y - py),
          GPoint(s2.x + px, s2.y - py),
          GPoint(s2.x - px, s2.y + py),
        };
        GPathInfo shadow_path_info = { .num_points = 4, .points = shadow_points };
        GPath *shadow_path = gpath_create(&shadow_path_info);
        graphics_context_set_fill_color(ctx, shadow_color);
        gpath_draw_filled(ctx, shadow_path);
        gpath_destroy(shadow_path);
  #endif  

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
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, outer_color);
  graphics_fill_circle(ctx, origin, settings.CentreSize); //started as 4
  
  graphics_context_set_fill_color(ctx, inner_color);
  graphics_fill_circle(ctx, origin, settings.InnerCentreSize); //started as 2

}


static void draw_major_tick (GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
      GPoint p1;
      GPoint p2;

      #ifdef PBL_ROUND
        p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 16 );
        p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 16 + length);
      #else
        if(settings.ForegroundShape){
          p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 16 );
          p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 16 + length);
        }
        else{
          GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
          GPoint edge = angle_to_rect_edge(origin, angle, r);
          int32_t dx = cos_lookup(DEG_TO_TRIGANGLE(angle));
          int32_t dy = sin_lookup(DEG_TO_TRIGANGLE(angle));
          p2 = GPoint(edge.x - (int)((dx * config.tick_inset_outer) / TRIG_MAX_ANGLE),
                            edge.y - (int)((dy * config.tick_inset_outer) / TRIG_MAX_ANGLE));
          p1 = angle_to_rounded_rect_edge(origin, angle, config.majortickrect_w, config.majortickrect_h, config.corner_radius_majortickrect);
        }
      #endif
 
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, p1, p2);
}

static void draw_minor_tick(GContext *ctx, int angle, GColor border_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
      GPoint p1;
      GPoint p2;

      #ifdef PBL_ROUND
          // The tick starts away from the center of the watch face.
          p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 8);
          // The tick ends closer to the edge.
          p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 );
      #else
        if(settings.ForegroundShape){
            p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - 8);
            p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 );
          }
          else{
            GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
            GPoint edge = angle_to_rect_edge(origin, angle, r);
            int32_t dx = cos_lookup(DEG_TO_TRIGANGLE(angle));
            int32_t dy = sin_lookup(DEG_TO_TRIGANGLE(angle));
            p2 = GPoint(edge.x - (int)((dx * config.tick_inset_outer) / TRIG_MAX_ANGLE),
                              edge.y - (int)((dy * config.tick_inset_outer) / TRIG_MAX_ANGLE));
            p1 = angle_to_rounded_rect_edge(origin, angle, config.minortickrect_w, config.minortickrect_h, config.corner_radius_minortickrect);
          }
      #endif

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 1); // A thin line for minor ticks
  graphics_draw_line(ctx, p1, p2);
}


#ifdef PBL_BW  //DON'T use FCTX a second time on Aplite: also use on Diorite and Flint as fctx is less efficient
static void update_logo_date_battery_fctx_layer (Layer *layer, GContext *ctx) {
  
  GRect bounds = layer_get_bounds(s_date_battery_logo_layer);

  //draw battery value
  if(settings.EnableBattery ){
     if(strcmp(settings.PosTop, "lo") == 0){
      int s_battery_level = battery_state_service_peek().charge_percent;
      char BatterytoDraw[6];
            snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

      if (settings.EnableBatteryLine) {
        GRect BatteryRect = GRect((bounds.size.w / 2) - 18, config.BatteryYOffset, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
      else{
        GRect BatteryRect = GRect((bounds.size.w / 2) - 18, config.BatteryYOffset + 4, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
    }

    if(strcmp(settings.PosBottom, "lo") == 0){
      int s_battery_level = battery_state_service_peek().charge_percent;
      char BatterytoDraw[6];
            snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

      if (settings.EnableBatteryLine) {
        GRect BatteryRect = GRect((bounds.size.w / 2) - 18, config.BatteryYOffset + (bounds.size.h/2) -8, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
      else{
        GRect BatteryRect = GRect((bounds.size.w / 2) - 18, config.BatteryYOffset + 4 + (bounds.size.h/2) -8, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
    }

    if(strcmp(settings.PosLeft, "lo") == 0){
      int s_battery_level = battery_state_service_peek().charge_percent;
      char BatterytoDraw[6];
            snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

      if (settings.EnableBatteryLine) {
        GRect BatteryRect = GRect(18, (bounds.size.h/2)  - 8 - 5, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
      else{
        GRect BatteryRect = GRect(18, (bounds.size.h/2) -8 - 5 + 4, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
    }

    if(strcmp(settings.PosRight, "lo") == 0){
      int s_battery_level = battery_state_service_peek().charge_percent;
      char BatterytoDraw[6];
            snprintf(BatterytoDraw,sizeof(BatterytoDraw),"%d",s_battery_level);

      if (settings.EnableBatteryLine) {
        GRect BatteryRect = GRect((bounds.size.w/2) + 18, (bounds.size.h/2)  - 8 - 5, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
      else{
        GRect BatteryRect = GRect((bounds.size.w/2) + 18, (bounds.size.h/2) -8 - 5 + 4, 36, 40);
        graphics_context_set_text_color(ctx, settings.BWDateColor);
        graphics_draw_text(ctx, BatterytoDraw, FontBattery, BatteryRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
    }
  }

  //draw weekday and date text
  if (settings.EnableDate ) {
    minutes = prv_tick_time->tm_min;
    hours = prv_tick_time->tm_hour % 12;
    ///use below for testing and for screenshots
       // minutes = 30;
       // hours = 9;

    int xPosition;
    int yPosition;
    int xOffset = config.xOffset;
    int yOffset = config.yOffset;
    int xWeekdayOffset = config.xWeekdayOffset;
    int yWeekdayDateOffset = config.yWeekdayDateOffset;
    int xDateOffset = config.xDateOffset;
    int xyMaskOffset = config.xyMaskOffset;
    int ShadowAndMaskWidth = config.ShadowAndMaskWidth;
    int ShadowAndMaskHeight = config.ShadowAndMaskHeight;
    int WeekdayWidth = config.WeekdayWidth;
    int DateWidth = config.DateWidth;
    int WeekdayDateHeight = config.WeekdayDateHeight;

    
    ////////Draw hour digits in one of 4 positions

    if(strcmp(settings.PosBottom, "hr") == 0){

       graphics_context_set_antialiased(ctx, true);

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

            GRect hour_rect = GRect(0, bounds.size.h * 3/4 - 15, bounds.size.w, 20);
             graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
            if(settings.DigitalHour){
             graphics_draw_text(ctx, hournow, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            }
            else{
             graphics_draw_text(ctx, mindraw, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
            }
   }
   
   if(strcmp(settings.PosTop, "hr") == 0){

         graphics_context_set_antialiased(ctx, true);

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

            GRect hour_rect = GRect(0, bounds.size.h/4 - 15, bounds.size.w, 20);
             graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
            if(settings.DigitalHour){
             graphics_draw_text(ctx, hournow, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            }
            else{
             graphics_draw_text(ctx, mindraw, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
            }
    }


    if(strcmp(settings.PosRight, "hr") == 0){
         graphics_context_set_antialiased(ctx, true);
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

            GRect hour_rect = GRect(bounds.size.w/2, bounds.size.h/2 - 15, bounds.size.w/2, 20);
             graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
            if(settings.DigitalHour){
             graphics_draw_text(ctx, hournow, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            }
            else{
             graphics_draw_text(ctx, mindraw, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
            }

    }
    
    if(strcmp(settings.PosLeft, "hr") == 0){

         graphics_context_set_antialiased(ctx, true);
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

            GRect hour_rect = GRect(0, bounds.size.h/2 - 15, bounds.size.w/2, 20);
             graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
             if(settings.DigitalHour){
             graphics_draw_text(ctx, hournow, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            }
            else{
             graphics_draw_text(ctx, mindraw, FontHour, hour_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
            }
    }

///////Draw date in one of 4 positions


    if(strcmp(settings.PosBottom, "dt") == 0){

          xPosition = config.BottomXPosition;
          yPosition = config.DateBottomYPosition;

          graphics_context_set_antialiased(ctx, true);

            const char * sys_locale = i18n_get_system_locale();
            char weekday[5];
            fetchwday(s_weekday, sys_locale, weekday);

            char weekdaydraw[10];
            snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

            char daynow[5];
            snprintf(daynow, sizeof(daynow), "%d", current_date);

            GRect WeekdayRect =
                GRect(xPosition + xOffset + xWeekdayOffset, yPosition + yOffset +yWeekdayDateOffset, WeekdayWidth, WeekdayDateHeight);

            GRect DateRect =
                GRect(xPosition + xOffset + xDateOffset, yPosition + yOffset +yWeekdayDateOffset, DateWidth, WeekdayDateHeight);

            graphics_context_set_text_color(ctx, settings.BWDateColor);
            graphics_draw_text(ctx, weekdaydraw, FontDate, WeekdayRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, daynow, FontDate, DateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
           

    }

    if(strcmp(settings.PosLeft, "dt") == 0){

          xPosition = config.LeftxPosition;
          yPosition = bounds.size.h/2;

          graphics_context_set_antialiased(ctx, true);

    

            const char * sys_locale = i18n_get_system_locale();
            char weekday[5];
            fetchwday(s_weekday, sys_locale, weekday);

            char weekdaydraw[10];
            snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

            char daynow[5];
            snprintf(daynow, sizeof(daynow), "%d", current_date);

            GRect WeekdayRect =
                GRect(xPosition + xOffset + xWeekdayOffset, yPosition + yOffset +yWeekdayDateOffset, WeekdayWidth, WeekdayDateHeight);

            GRect DateRect =
                GRect(xPosition + xOffset + xDateOffset, yPosition + yOffset +yWeekdayDateOffset, DateWidth, WeekdayDateHeight);

            graphics_context_set_text_color(ctx, settings.BWDateColor);
            graphics_draw_text(ctx, weekdaydraw, FontDate, WeekdayRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, daynow, FontDate, DateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    if(strcmp(settings.PosRight, "dt") == 0){

         xPosition = bounds.size.w/2;
         yPosition = bounds.size.h/2;

         graphics_context_set_antialiased(ctx, true);

    

            const char * sys_locale = i18n_get_system_locale();
            char weekday[5];
            fetchwday(s_weekday, sys_locale, weekday);

            char weekdaydraw[10];
            snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

            char daynow[5];
            snprintf(daynow, sizeof(daynow), "%d", current_date);


            GRect WeekdayRect =
                GRect(xPosition + xOffset + xWeekdayOffset, yPosition + yOffset +yWeekdayDateOffset, WeekdayWidth, WeekdayDateHeight);

            GRect DateRect =
                GRect(xPosition + xOffset + xDateOffset, yPosition + yOffset +yWeekdayDateOffset, DateWidth, WeekdayDateHeight);


            graphics_context_set_text_color(ctx, settings.BWDateColor);
            graphics_draw_text(ctx, weekdaydraw, FontDate, WeekdayRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, daynow, FontDate, DateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

      }

      if(strcmp(settings.PosTop, "dt") == 0){

        xPosition = config.BottomXPosition;
        yPosition = bounds.size.h/4;

        graphics_context_set_antialiased(ctx, true);

    

            const char * sys_locale = i18n_get_system_locale();
            char weekday[5];
            fetchwday(s_weekday, sys_locale, weekday);

            char weekdaydraw[10];
            snprintf(weekdaydraw, sizeof(weekdaydraw), "%s", weekday);

            char daynow[5];
            snprintf(daynow, sizeof(daynow), "%d", current_date);

            GRect WeekdayRect =
                GRect(xPosition + xOffset + xWeekdayOffset, yPosition + yOffset +yWeekdayDateOffset, WeekdayWidth, WeekdayDateHeight);

            GRect DateRect =
                GRect(xPosition + xOffset + xDateOffset, yPosition + yOffset +yWeekdayDateOffset, DateWidth, WeekdayDateHeight);

            graphics_context_set_text_color(ctx, settings.BWDateColor);
            graphics_draw_text(ctx, weekdaydraw, FontDate, WeekdayRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, daynow, FontDate, DateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      }
        
  }

//////////draw logo at top or bottom (4 positions)

  if (settings.EnableLogo) {

    if(strcmp(settings.PosRight, "lo") == 0){
      //draw logo text
      GRect LogoRect = GRect((bounds.size.w / 2)+4, bounds.size.h/2 + 3 , bounds.size.w / 2-8, 40);

      char logodraw [20];
      snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);

      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, logodraw, FontLogo, LogoRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    if(strcmp(settings.PosLeft, "lo") == 0){
      //draw logo text
      GRect LogoRect = GRect(4, bounds.size.h/2 + 3 , bounds.size.w / 2-8, 40);

      char logodraw [20];
      snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);

      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, logodraw, FontLogo, LogoRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    if(strcmp(settings.PosTop, "lo") == 0){
      //draw logo text
      GRect LogoRect = GRect((bounds.size.w / 2) - 34, config.LogoYOffset - 1 , 68, 40);

      char logodraw [20];
      snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);

      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, logodraw, FontLogo, LogoRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    if(strcmp(settings.PosBottom, "lo") == 0){
      //draw logo text
      GRect LogoRect = GRect((bounds.size.w / 2) - 34, config.LogoYOffset - 1 + (bounds.size.h/2) - 8, 68, 40);

      char logodraw [20];
      snprintf(logodraw, sizeof(logodraw), "%s", settings.LogoText);

      graphics_context_set_text_color(ctx, settings.BWDateColor);
      graphics_draw_text(ctx, logodraw, FontLogo, LogoRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

  }

if (!clock_is_24h_style()){

    if(strcmp(settings.PosTop, "ap") == 0){
      char local_ampm_string[5];
      strftime(local_ampm_string, sizeof(local_ampm_string), "%p", prv_tick_time);

        bool is_am = (prv_tick_time->tm_hour < 12);

        GRect ampm_rect = GRect(0, bounds.size.h/4 - 6, bounds.size.w, 6);
        graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
        graphics_draw_text(ctx, local_ampm_string, FontBattery, ampm_rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

    }

    if(strcmp(settings.PosBottom, "ap") == 0){
      char local_ampm_string[5];
      strftime(local_ampm_string, sizeof(local_ampm_string), "%p", prv_tick_time);

        bool is_am = (prv_tick_time->tm_hour < 12);

        GRect ampm_rect = GRect(0, bounds.size.h * 3/4 - 6, bounds.size.w, 6);
        graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
        graphics_draw_text(ctx, local_ampm_string, FontBattery, ampm_rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

    }

    if(strcmp(settings.PosRight, "ap") == 0){
      char local_ampm_string[5];
      strftime(local_ampm_string, sizeof(local_ampm_string), "%p", prv_tick_time);

        bool is_am = (prv_tick_time->tm_hour < 12);
      
        GRect ampm_rect = GRect(bounds.size.w/2, bounds.size.h/2 - 6, bounds.size.w/2, 6);
        graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
        graphics_draw_text(ctx, local_ampm_string, FontBattery, ampm_rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

      }
    
      if(strcmp(settings.PosLeft, "ap") == 0){
      char local_ampm_string[5];
      strftime(local_ampm_string, sizeof(local_ampm_string), "%p", prv_tick_time);

        bool is_am = (prv_tick_time->tm_hour < 12);
       
        GRect ampm_rect = GRect(0, bounds.size.h/2 - 6, bounds.size.w/2, 6);
        graphics_context_set_text_color(ctx, settings.BWHourDigitsColor);
        graphics_draw_text(ctx, local_ampm_string, FontBattery, ampm_rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

      }
    }
}

static void layer_update_proc_battery_line(Layer *layer, GContext *ctx) {
    // If neither element is enabled in config, stop.
    if (!settings.EnableBattery && !settings.EnableBatteryLine) {
        return;
    }

    int s_battery_level = battery_state_service_peek().charge_percent;

    // Draw battery line
    if(strcmp(settings.PosTop, "lo") == 0){
    if (settings.EnableBatteryLine) {
        int width_rect = (s_battery_level * config.battery_line) / 100;
        int rect_x_pos = (bounds.size.w/2) - (width_rect/2);

        GRect BatteryLineRect = GRect(rect_x_pos,config.BatteryLineYOffset,width_rect, 2);
        graphics_context_set_antialiased(ctx, true);
        graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.BatteryLineColor));
        graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);
    }
    }

    if(strcmp(settings.PosBottom, "lo") == 0){
    if (settings.EnableBatteryLine) {
        int width_rect = (s_battery_level * config.battery_line) / 100;
        int rect_x_pos = (bounds.size.w/2) - (width_rect/2);

        GRect BatteryLineRect = GRect(rect_x_pos,config.BatteryLineYOffset +config.BatteryLineYOffset2,width_rect, 2);
        graphics_context_set_antialiased(ctx, true);
        graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.BatteryLineColor));
        graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);
    }
    }

    if(strcmp(settings.PosLeft, "lo") == 0){
    if (settings.EnableBatteryLine) {
        int width_rect = (s_battery_level * config.battery_line) / 100;
        int rect_x_pos = (bounds.size.w/4) - (width_rect/2);

        GRect BatteryLineRect = GRect(rect_x_pos,(bounds.size.h/2),width_rect, 2);
        graphics_context_set_antialiased(ctx, true);
        graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.BatteryLineColor));
        graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);
    }
    }

    if(strcmp(settings.PosRight, "lo") == 0){
    if (settings.EnableBatteryLine) {
        int width_rect = (s_battery_level * config.battery_line) / 100;
        int rect_x_pos = (bounds.size.w*3/4) - (width_rect/2);

        GRect BatteryLineRect = GRect(rect_x_pos,(bounds.size.h/2),width_rect, 2);
        graphics_context_set_antialiased(ctx, true);
        graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.BatteryLineColor));
        graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);
    }
    }
}

#else   //use FCTX to antialise the digits better on all colour watches, still refers to B&W in case I change my mind later on non-APLITE watches

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

static void draw_complication_border(FContext *fctxp, GPoint center) {
  if (settings.ComplicationBorderColor.argb == settings.BackgroundColor1.argb)
    return;

  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  int radius = 5 * bounds.size.w / 32 + config.ComplicationBorderAdj + orbitadj;
  graphics_context_set_antialiased(fctxp->gctx, true);
  graphics_context_set_stroke_color(fctxp->gctx, settings.ComplicationBorderColor);
  graphics_context_set_stroke_width(fctxp->gctx, 1);
  graphics_draw_circle(fctxp->gctx, center, radius);

  if (settings.ShadowOn && settings.ComplicationShadowColor.argb != settings.BackgroundColor1.argb) {
    GRect shadow_rect = GRect(center.x - radius, center.y - radius + 1, 2 * radius, 2 * radius);
    graphics_context_set_stroke_color(fctxp->gctx, settings.ComplicationShadowColor);
    graphics_draw_arc(fctxp->gctx, shadow_rect, GOvalScaleModeFitCircle, TRIG_QUARTER_ANGLE, TRIG_HALF_ANGLE + TRIG_QUARTER_ANGLE);
  }
}

static void render_hour_digits_fctx(FContext *fctxp, int angle_native) {
  fctx_set_fill_color(fctxp, PBL_IF_BW_ELSE(settings.BWHourDigitsColor, settings.HourDigitsColor));
  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  GPoint rel_pos = polar_to_point_native(angle_native, bounds.size.w / 4 + config.ComplicationDistanceAdj - orbitadj);
  GPoint abs_pos = relative_gpoint_to_absolute(&rel_pos);
  FPoint hour_pos = gpoint_to_fpoint(&abs_pos);

  draw_complication_border(fctxp, abs_pos);

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
  if(settings.DigitalHour){
    fctx_draw_string(fctxp, hournow, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
  }
  else{
    fctx_draw_string(fctxp, mindraw, Date_Font, GTextAlignmentCenter, FTextAnchorMiddle);
  }
  fctx_end_fill(fctxp);
}

static void render_ampm_fctx(FContext *fctxp, int angle_native) {
  fctx_set_fill_color(fctxp, PBL_IF_BW_ELSE(settings.BWHourDigitsColor, settings.HourDigitsColor));

  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  GPoint rel_pos = polar_to_point_native(angle_native, bounds.size.w / 4 + config.ComplicationDistanceAdj - orbitadj);
  GPoint abs_pos = relative_gpoint_to_absolute(&rel_pos);
  FPoint ampm_pos = gpoint_to_fpoint(&abs_pos);

  draw_complication_border(fctxp, abs_pos);

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
  #elif defined (PBL_PLATFORM_GABBRO)
    #define LOGO_WRAP_AT 18
  #else
    #define LOGO_WRAP_AT 12
  #endif

  const int logo_top_margin = 2;
  const int logo_line_spacing = 2;
  fctx_set_fill_color(fctxp, PBL_IF_BW_ELSE(settings.BWDateColor, settings.DateColor));

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

  fctx_set_fill_color(fctxp, PBL_IF_BW_ELSE(settings.BWDateColor, settings.DateColor));

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
    int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
    GPoint rel_pos = polar_to_point_native(angle_native, bounds.size.w / 4 + config.ComplicationDistanceAdj - orbitadj);
    GPoint abs_pos = relative_gpoint_to_absolute(&rel_pos);
    render_pos = gpoint_to_fpoint(&abs_pos);

    draw_complication_border(fctxp, abs_pos);
  }

  if (settings.EnableBattery)
    render_battery_pct_fctx(fctxp, render_pos);
  if (settings.EnableLogo)
    render_logo_fctx(fctxp, render_pos);
}

static void render_date_fctx(FContext *fctxp, int angle_native) {
  fctx_set_fill_color(fctxp, PBL_IF_BW_ELSE(settings.BWDateColor, settings.DateColor));

  minutes = prv_tick_time->tm_min;
  hours = prv_tick_time->tm_hour % 12;

  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  GPoint rel_pos = polar_to_point_native(angle_native, bounds.size.w / 4 + config.ComplicationDistanceAdj - orbitadj);
  GPoint abs_pos = relative_gpoint_to_absolute(&rel_pos);
  FPoint weekday_pos = gpoint_to_fpoint(&abs_pos);
  FPoint date_pos = weekday_pos;

  draw_complication_border(fctxp, abs_pos);

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

static inline int get_base_angle() {
  const int top_angle = -TRIG_QUARTER_ANGLE;
  return settings.OrbitComplications ? hand_angle_native : top_angle;
}

static void update_logo_date_battery_fctx_layer (Layer *layer, GContext *ctx) {
  int base_angle = get_base_angle();

  APP_LOG(APP_LOG_LEVEL_INFO, "update_logo_date_battery_fctx_layer");
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

    if (strcmp(currSetting, "hr") == 0) {
      render_hour_digits_fctx(&fctx, *curr_angle);
    } else if(strcmp(currSetting, "lo") == 0) {
      render_logo_battery_fctx(&fctx, *curr_angle);
    } else if (strcmp(currSetting, "dt") == 0) {
      if (settings.EnableDate)
        render_date_fctx(&fctx, *curr_angle);
    } else if (strcmp(currSetting, "ap") == 0) {
      if (!clock_is_24h_style())
        render_ampm_fctx(&fctx, *curr_angle);
    }
  }

  fctx_deinit_context(&fctx);
}

static void render_battery_line(GContext *ctx, int angle_native, int s_battery_level) {
  int width_rect = (s_battery_level * config.battery_line) / 100;
  int orbitadj = settings.OrbitComplications ? config.ComplicationOrbitSizeAdj : 0;
  GPoint line_center = polar_to_point_native(angle_native, bounds.size.w/4 + config.ComplicationDistanceAdj - orbitadj);
  line_center.x += bounds.size.w / 2;
  line_center.y += bounds.size.h / 2;

  GRect BatteryLineRect = GRect(line_center.x - width_rect/2, line_center.y, width_rect, 2);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.BatteryLineColor));
  graphics_fill_rect(ctx,BatteryLineRect, 1, GCornersBottom);  
}

static void layer_update_proc_battery_line(Layer *layer, GContext *ctx) {
  // If neither element is enabled in config, stop.
  if (!settings.EnableBattery && !settings.EnableBatteryLine) {
      return;
  }

  int s_battery_level = battery_state_service_peek().charge_percent;

  int base_angle = get_base_angle();

  // Draw battery line
  int startidx = settings.OrbitComplications ? 1 : 0;
  char* compSettings[] = { settings.PosTop, settings.PosRight, settings.PosBottom, settings.PosLeft, NULL };
  int side_angle = settings.OrbitComplications ? TRIG_7_32_ANGLE : TRIG_QUARTER_ANGLE;
  int angles[] = { base_angle, base_angle + side_angle, base_angle + TRIG_HALF_ANGLE, base_angle - side_angle, 0 };
  int *curr_angle = &angles[startidx];
  for (char** setting = compSettings + startidx;
      *setting;
      curr_angle++, setting++) {

    char* currSetting = *setting;

    if (strcmp(currSetting, "lo") == 0) {
      if (settings.EnableBatteryLine) {
        render_battery_line(ctx, *curr_angle, s_battery_level);
      }
    }
  }
}

#endif

//Update procedure for the Bluetooth Icon (shows when disconnected) layer
static void layer_update_proc_bt(Layer * layer, GContext * ctx){
   minutes = prv_tick_time->tm_min;
   hours = prv_tick_time->tm_hour % 12;

//use this for testing
   // minutes = 30;
   // hours = 9;

      int xPosition;
      int yPosition;
      int textboxwidth;
      int BTIconYOffset;

        // Bottom position
      
      #ifdef PBL_BW
              xPosition = config.BottomXPosition + 2;
              yPosition = config.BTQTBottomYPosition;
              textboxwidth = config.ShadowAndMaskWidth/2;
              BTIconYOffset = config.BTIconYOffset;
      #elif defined (PBL_PLATFORM_BASALT)
              xPosition = config.BottomXPosition + 2;
              yPosition = config.BTQTBottomYPosition;
              textboxwidth = config.ShadowAndMaskWidth/2;
              BTIconYOffset = config.BTIconYOffset;
      #else
            if(quiet_time_is_active()){
              xPosition = config.BottomXPosition + 2;
              yPosition = config.BTQTBottomYPosition;
              textboxwidth = config.ShadowAndMaskWidth/2;
              BTIconYOffset = config.BTIconYOffset;
            }
            else{
              xPosition = config.BottomXPosition;
              yPosition = config.BTQTBottomYPosition;
              textboxwidth = config.ShadowAndMaskWidth;
              BTIconYOffset = config.BTIconYOffset;
            }
        #endif
     

  GRect BTIconRect =
    GRect(xPosition + config.xOffset + config.BTIconXOffset2, yPosition + config.yOffset + BTIconYOffset + config.BTIconYOffset2, textboxwidth, 20);


#ifdef PBL_COLOR
 graphics_context_set_text_color(ctx, settings.BTQTColor);
 #else
  graphics_context_set_text_color(ctx, settings.BWBTQTColor);
 #endif

 graphics_context_set_antialiased(ctx, true);
 graphics_draw_text(ctx, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);


}

//Update procedure for the QT Icon layer (shows when Quiet time is active)
static void layer_update_proc_qt(Layer * layer, GContext * ctx){

   minutes = prv_tick_time->tm_min;
   hours = prv_tick_time->tm_hour % 12;

//use this for testing
   // minutes = 30;
   // hours = 9;

      int xPosition;
      int yPosition;
      int textboxwidth;
      int QTIconYOffset;


   
        // Bottom position
      #ifdef PBL_BW
        xPosition = config.BottomXPosition;
        yPosition = config.BTQTBottomYPosition;
        textboxwidth = config.ShadowAndMaskWidth;
        QTIconYOffset = 0 - config.QTIconYOffset;
      #elif defined (PBL_PLATFORM_BASALT)
        xPosition = config.BottomXPosition;
        yPosition = config.BTQTBottomYPosition;
        textboxwidth = config.ShadowAndMaskWidth;
        QTIconYOffset = 0 - config.QTIconYOffset;
      #else
       if(connection_service_peek_pebble_app_connection()){

        xPosition = config.BottomXPosition;
        yPosition = config.BTQTBottomYPosition -1;
        textboxwidth = config.ShadowAndMaskWidth;
        QTIconYOffset = 0 - config.QTIconYOffset;
       }
      else{
        xPosition = config.BottomXPosition + config.ShadowAndMaskWidth/2 - 2;
        yPosition = config.BTQTBottomYPosition -1 ;
        textboxwidth = config.ShadowAndMaskWidth/2;
        QTIconYOffset = 0 - config.QTIconYOffset;
      }
      #endif
     

  GRect QTIconRect =
    GRect(xPosition + config.xOffset + config.QTIconXOffset2, yPosition + config.yOffset + QTIconYOffset + config.QTIconYOffset2, textboxwidth, 20);

 quiet_time_icon(); //checks whether quiet time is active

 #ifdef PBL_COLOR
  graphics_context_set_text_color(ctx, settings.BTQTColor);
  #else
   graphics_context_set_text_color(ctx, settings.BWBTQTColor);
  #endif
  graphics_context_set_antialiased(ctx, true);
  graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);

}

int calculate_hand_angle(struct tm *prv_tm) {
  // Using native trig angles since we're dealing with small fractions of degrees
  int angle;

  if (settings.DigitalHour) {
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
          PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.MinutesHandColor));
      draw_hand_center(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.MinutesHandColor), PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));
  #else
      if(settings.ForegroundShape){
          draw_line_hand(ctx, hand_angle_native,
              bounds.size.w/2 - config.analogue_hand_a,
              settings.BackLen,
              PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.MinutesHandColor));
          draw_hand_center(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.MinutesHandColor), PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));
      }
      else{
          draw_line_hand(ctx, hand_angle_native,
              bounds.size.w/2 - config.analogue_hand_c,
              settings.BackLen,
              PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.MinutesHandColor));
          draw_hand_center(ctx, PBL_IF_BW_ELSE(settings.BWMinHandBatLineColor, settings.MinutesHandColor), PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1));
      }
  #endif

}

///update procedure for background
static void bg_update_proc(Layer *layer, GContext *ctx) {

  GRect bounds = layer_get_bounds(layer);

  GRect Background =
       GRect(0, 0, bounds.size.w, bounds.size.h);

   graphics_context_set_fill_color(ctx,PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1) );
   graphics_fill_rect(ctx, Background,0,GCornersAll);

  if(settings.showMinorTick){
        for (int i = 0; i < 60; i++) {
        //if (i % 5 == 0) continue;
        int angle = i * 6;
        draw_minor_tick(ctx, angle, PBL_IF_BW_ELSE(settings.BWMajorTickColor, settings.MinorTickColor));
      }
    }

  if(settings.showMajorTick){
      for (int i = 0; i < 12; i++) {
        if (i == 6 || i == 12 || i == 3 || i == 9 || i == 0) {
        int angle = i * 30 - 90;
        draw_major_tick(ctx, angle, 16, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.MajorTickColor), PBL_IF_BW_ELSE(settings.BWMajorTickColor, settings.MajorTickColor));
      }

      else {
        //if (i == 6 || i == 12) continue;
        int angle = i * 30 - 90;
        draw_major_tick(ctx, angle, 16, PBL_IF_BW_ELSE(settings.BWBackgroundColor1, settings.BackgroundColor1), PBL_IF_BW_ELSE(settings.BWMajorTickColor, settings.MinorTickColor));
      }
    }
    }

}


static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);
  current_date = prv_tick_time->tm_mday;
  s_weekday = prv_tick_time->tm_wday;
  seconds = prv_tick_time->tm_sec;
  #if defined(SUB_MINUTE_USE_TICK)
  sub_minute_interval = settings.SmoothMinuteHand ? prv_tick_time->tm_sec / settings.MinuteHandUpdateIntervalSec : 0;
  #endif
  minutes = prv_tick_time->tm_min;
  hours = prv_tick_time->tm_hour % 12;
  s_hours = prv_tick_time->tm_hour;
  hand_angle_native = calculate_hand_angle(prv_tick_time);

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  // Load fctx ffonts
  Date_Font =  ffont_create_from_resource(RESOURCE_ID_FONT_DATE_FCTX);
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_X));
  //non-fctx custom fonts for B&W screens
  #ifdef PBL_BW
  FontDate = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_9));
  FontBattery = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_10));
  FontLogo = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_8));
  FontHour = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_24));
  #endif

  // Subscribe to the connection service to get Bluetooth status updates.
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_vibe_icon
  });

#if defined(SUB_MINUTE_USE_APPTIMER)
  sub_minute_timer = app_timer_register_(1000 * settings.MinuteHandUpdateIntervalSec, apptimer_handler, NULL);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

#elif defined (SUB_MINUTE_USE_TICK)
  tick_timer_service_subscribe(settings.SmoothMinuteHand ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
#endif

  //create layers
  s_bg_layer = layer_create(bounds);
  s_dial_layer = layer_create(bounds);
  s_canvas_qt_icon = layer_create(bounds);
     quiet_time_icon();
  s_canvas_bt_icon = layer_create(bounds);
    bool is_connected = connection_service_peek_pebble_app_connection();
    layer_set_hidden(s_canvas_bt_icon, is_connected);
  s_canvas_battery = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  s_date_battery_logo_layer = layer_create(bounds);

  // Change the order here
  layer_add_child(window_layer, s_bg_layer); //backforound, circles, major tick shoadow &tickmask
  layer_add_child(window_layer, s_canvas_bt_icon);
  layer_add_child(window_layer, s_canvas_qt_icon);
  layer_add_child(window_layer, s_date_battery_logo_layer); //fctx version of text
  layer_add_child(window_layer, s_canvas_battery); //battery line
  layer_add_child(window_layer, s_canvas_layer);  //hour and minute hands
 
  bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_canvas_bt_icon, layer_update_proc_bt);
  layer_set_update_proc(s_canvas_qt_icon, layer_update_proc_qt);
  layer_set_update_proc(s_date_battery_logo_layer, update_logo_date_battery_fctx_layer);
  layer_set_update_proc(s_canvas_battery, layer_update_proc_battery_line);
  layer_set_update_proc(s_canvas_layer, hour_min_hands_canvas_update_proc);
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
  layer_destroy(s_bg_layer);
  layer_destroy(s_dial_layer);
  layer_destroy(s_canvas_battery);
  layer_destroy(s_canvas_bt_icon);
  layer_destroy(s_canvas_qt_icon);
  layer_destroy(s_date_battery_logo_layer);
  ffont_destroy(Date_Font);
  #ifdef PBL_BW
  fonts_unload_custom_font(FontDate);
  fonts_unload_custom_font(FontBattery);
  fonts_unload_custom_font(FontLogo);
  fonts_unload_custom_font(FontHour);
  #endif
  fonts_unload_custom_font(FontBTQTIcons);
}

static void prv_init(void) {
  prv_load_settings();

  // Open AppMessage and set the message handler
  app_message_open(512, 512);
  app_message_register_inbox_received(prv_inbox_received_handler);

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
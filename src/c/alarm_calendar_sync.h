#pragma once
#include <pebble.h>

// ----
// AppMessage keys used by the Monologue Orbital phone companion
// (see PebbleMessageKeys.kt in lnbot/monologue-orbital-companion).
// These are raw uint32 identifiers that must agree EXACTLY with the phone side.
// They are deliberately NOT added to package.json "messageKeys", because the
// auto-generated MESSAGE_KEY_* constants are assigned 10000 + array index and
// would not match the companion's 110/111/112. We define them explicitly here.
// ----
#define ALARM_CAL_SYNC_REQUEST  110  // uint8 flag, watch -> phone (request re-sync)
#define ALARM_CAL_SYNC_ALARM    111  // uint32, phone -> watch (next alarm epoch)
#define ALARM_CAL_SYNC_CALENDAR 112  // uint8[], phone -> watch (LE uint32 epochs, 0-terminated)
#define ALARM_CAL_SYNC_TIMER    113  // uint32, phone -> watch (active timer completion epoch)

// A calendar payload is a byte array of consecutive little-endian uint32 values
// ending with a single zero uint32 value. Max 17 uint32 total, i.e. <= 16 events.
#define MAX_CALENDAR_EVENTS 16

// Data is considered stale after this many seconds and a re-sync is requested.
#define SYNC_DATA_FRESH_SECONDS (30 * 60)

// Persist key for the alarm/calendar sync data (SETTINGS_KEY 125 is used by settings).
#define SYNC_DATA_PERSIST_KEY 126

// Update this when SyncData struct changes
#define SYNC_DATA_VERSION 1

// In-memory copy of the latest synced alarm/calendar data.
typedef struct {
  int      version;                     // version of this struct (for future-proofing)
  uint32_t alarm_epoch;                 // 0 = no alarm set / cleared
  uint32_t timer_epoch;                 // 0 = no timer running / cleared
  uint32_t calendar[MAX_CALENDAR_EVENTS]; // next event epochs (0-padded beyond count)
  int      event_count;                 // number of valid entries in calendar[]
  time_t   last_updated;                // time(NULL) when data was last received
} SyncData;

// Initialise the module: registers the inbox handler (which processes the sync
// keys and then forwards the dictionary to `forward`), and loads persisted data.
// `forward` is the watchface's existing settings inbox handler.
void alarm_calendar_sync_init(AppMessageInboxReceived forward);

// Enable/disable listening for alarm/calendar sync messages.
void alarm_calendar_sync_set_enabled(bool enabled);

// If enabled and the stored data is older than SYNC_DATA_FRESH_SECONDS, send a
// manual SyncRequest to the companion. Call from window load and the tick handler.
void alarm_calendar_sync_maybe_request_update(void);

// Accessors for the in-memory data (available to the watchface).
uint32_t alarm_calendar_sync_get_alarm(void);
uint32_t alarm_calendar_sync_get_timer(void);
uint32_t alarm_calendar_sync_get_event_at(int index);
int      alarm_calendar_sync_get_event_count(void);
time_t   alarm_calendar_sync_last_updated(void);

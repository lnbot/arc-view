#include "alarm_calendar_sync.h"

// ---------------------------------------------------------------------------
// In-memory + persisted state
// ---------------------------------------------------------------------------

static SyncData s_data;
static bool s_enabled = false;
static AppMessageInboxReceived s_forward = NULL;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void prv_load_data(void) {
  // Read persisted data; if nothing is stored (or the key is missing) the
  // struct stays zeroed so a fresh SyncRequest will be triggered immediately.
  if (persist_exists(SYNC_DATA_PERSIST_KEY)) {
    persist_read_data(SYNC_DATA_PERSIST_KEY, &s_data, sizeof(s_data));
  }
}

static void prv_save_data(void) {
  persist_write_data(SYNC_DATA_PERSIST_KEY, &s_data, sizeof(s_data));
}

static void prv_send_sync_request(void) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if (result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: outbox_begin failed (%d)", (int)result);
    return;
  }
  // uint8 flag, key 110 -> companion re-reads and pushes alarm + calendar.
  dict_write_uint8(iter, ALARM_CAL_SYNC_REQUEST, 1);
  result = app_message_outbox_send();
  if (result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: outbox_send failed (%d)", (int)result);
  }
}

// Decode a SyncCalendar byte array of little-endian uint32 values ending with a
// zero uint32 terminator (max 17 uint32 values counting the terminator).
static void prv_decode_calendar(const uint8_t *data, uint32_t length) {
  int count = 0;
  uint32_t offset = 0;

  while (offset + 4 <= length && count < MAX_CALENDAR_EVENTS) {
    // The companion stores each uint32 in little-endian byte order, so on a
    // little-endian host a direct cast suffices; on big-endian hosts the value
    // must be byte-swapped back.
    uint32_t v = *(const uint32_t *)(data + offset);
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    v = __builtin_bswap32(v);
    #endif
    offset += 4;

    // The list is terminated by a zero uint32 value, which is not an event, so
    // stop decoding when we encounter it.
    if (v == 0) {
      break;
    }

    s_data.calendar[count++] = v;
  }

  s_data.event_count = count;

  // Zero-pad any remaining calendar slots beyond the events we stored.
  for (int i = count; i < MAX_CALENDAR_EVENTS; i++) {
    s_data.calendar[i] = 0;
  }
}

// ---------------------------------------------------------------------------
// Inbox handler
// ---------------------------------------------------------------------------

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  (void)context;

  if (s_enabled) {
    Tuple *alarm_t = dict_find(iter, ALARM_CAL_SYNC_ALARM);
    Tuple *calendar_t = dict_find(iter, ALARM_CAL_SYNC_CALENDAR);
    bool updated = false;

    if (alarm_t) {
      s_data.alarm_epoch = alarm_t->value->uint32;
      updated = true;
    }

    if (calendar_t && calendar_t->type == TUPLE_BYTE_ARRAY) {
      prv_decode_calendar(calendar_t->value->data, calendar_t->length);
      updated = true;
    }

    if (updated) {
      s_data.last_updated = time(NULL);
      prv_save_data();
    }
  }

  // Forward the dictionary to the watchface's settings handler (only one inbox
  // handler can be registered via the SDK, so we act as its dispatcher).
  if (s_forward) {
    s_forward(iter, context);
  }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void alarm_calendar_sync_init(AppMessageInboxReceived forward) {
  s_forward = forward;
  prv_load_data();

  // Open AppMessage and register our inbox handler. Buffers are sized to match
  // the original settings traffic (512 bytes each); the sync payload is tiny
  // (<=68 bytes) and fits easily.
  app_message_open(512, 512);
  app_message_register_inbox_received(prv_inbox_received_handler);
}

void alarm_calendar_sync_set_enabled(bool enabled) {
  s_enabled = enabled;
}

void alarm_calendar_sync_maybe_request_update(void) {
  if (!s_enabled) {
    return;
  }

  time_t now = time(NULL);
  if (now - s_data.last_updated >= SYNC_DATA_FRESH_SECONDS) {
    APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: data stale, requesting re-sync");
    prv_send_sync_request();
  }
}

uint32_t alarm_calendar_sync_get_alarm(void) {
  return s_data.alarm_epoch;
}

uint32_t alarm_calendar_sync_get_event_at(int index) {
  if (index < 0 || index >= MAX_CALENDAR_EVENTS) {
    return 0;
  }
  return s_data.calendar[index];
}

int alarm_calendar_sync_get_event_count(void) {
  return s_data.event_count;
}

time_t alarm_calendar_sync_last_updated(void) {
  return s_data.last_updated;
}

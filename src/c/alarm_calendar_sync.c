#include "alarm_calendar_sync.h"
#include "message_keys.auto.h"

// ---------------------------------------------------------------------------
// In-memory + persisted state
// ---------------------------------------------------------------------------

static SyncData s_data;
static bool s_enabled = false;
static bool s_alarm_pin = false;
static bool s_timer_pin = false;
static AppMessageInboxReceived s_forward = NULL;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void prv_load_data(void) {
  // Read persisted data; if nothing is stored (or the key is missing) the
  // struct stays zeroed so a fresh SyncRequest will be triggered immediately.
  if (persist_exists(SYNC_DATA_PERSIST_KEY)) {
    persist_read_data(SYNC_DATA_PERSIST_KEY, &s_data, sizeof(s_data));

    if (s_data.version != SYNC_DATA_VERSION) {
      APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: data version mismatch (%d != %d), clearing", s_data.version, SYNC_DATA_VERSION);
      memset(&s_data, 0, sizeof(s_data));
      s_data.version = SYNC_DATA_VERSION;
    }
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

// Tell the JS companion to insert (PIN_CREATE) or remove (PIN_DELETE) a user
// timeline pin for the given target (alarm/timer). `value` is the new epoch for
// a create (ignored for a delete). The companion keys each pin by a fixed id,
// so a create with a changed timestamp replaces the previous pin for that id,
// i.e. the old pin is removed and the new one placed.
static void prv_send_pin_command(uint32_t command, uint32_t target, uint32_t value) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if (result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: pin outbox_begin failed (%d)", (int)result);
    return;
  }
  dict_write_uint32(iter, command, value);
  dict_write_uint8(iter, MESSAGE_KEY_PinTarget, (uint8_t)target);
  result = app_message_outbox_send();
  if (result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: pin outbox_send failed (%d)", (int)result);
  }
}

static void prv_sync_alarm_pin(void) {
  if (s_data.alarm_epoch != 0) {
    prv_send_pin_command(MESSAGE_KEY_PinCreate, ALARM_CAL_SYNC_PIN_TARGET_ALARM,
                         s_data.alarm_epoch);
  } else {
    // Timestamp cleared -> just remove the old pin.
    prv_send_pin_command(MESSAGE_KEY_PinDelete, ALARM_CAL_SYNC_PIN_TARGET_ALARM, 0);
  }
}

static void prv_sync_timer_pin(void) {
  if (s_data.timer_epoch != 0) {
    prv_send_pin_command(MESSAGE_KEY_PinCreate, ALARM_CAL_SYNC_PIN_TARGET_TIMER,
                         s_data.timer_epoch);
  } else {
    // Timestamp cleared -> just remove the old pin.
    prv_send_pin_command(MESSAGE_KEY_PinDelete, ALARM_CAL_SYNC_PIN_TARGET_TIMER, 0);
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
    uint32_t start = *(const uint32_t *)(data + offset);
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    start = __builtin_bswap32(start);
    #endif
    offset += 4;

    // The list is terminated by a zero uint32 value, which is not an event, so
    // stop decoding when we encounter it.
    if (start == 0 || offset + 4 > length) {
      break;
    }

    uint32_t end = *(const uint32_t *)(data + offset);
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    end = __builtin_bswap32(end);
    #endif
    offset += 4;

    // This shouldn't happen, but just in case...
    if (end == 0) {
      break;
    }

    if (start > end)
      APP_LOG(APP_LOG_LEVEL_WARNING, "prv_decode_calendar: start > end (%u > %u)", start, end);

    s_data.calendar[count++] = (CalendarEvent){ start, end };
  }

  s_data.event_count = count;

  // Zero-pad any remaining calendar slots beyond the events we stored.
  for (int i = count; i < MAX_CALENDAR_EVENTS; i++) {
    s_data.calendar[i] = (CalendarEvent){ 0, 0 };
  }
}

// ---------------------------------------------------------------------------
// Inbox handler
// ---------------------------------------------------------------------------

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  (void)context;

  if (s_enabled) {
    Tuple *alarm_t = dict_find(iter, ALARM_CAL_SYNC_ALARM);
    Tuple *timer_t = dict_find(iter, ALARM_CAL_SYNC_TIMER);
    Tuple *calendar_t = dict_find(iter, ALARM_CAL_SYNC_CALENDAR);
    bool updated = false;

    // Capture the pre-update values so we can push pins only on a real change.
    uint32_t old_alarm = s_data.alarm_epoch;
    uint32_t old_timer = s_data.timer_epoch;

    if (alarm_t) {
      s_data.alarm_epoch = alarm_t->value->uint32;
      updated = true;
    }

    if (timer_t) {
      s_data.timer_epoch = timer_t->value->uint32;
      updated = true;
    }

    if (calendar_t && calendar_t->type == TUPLE_BYTE_ARRAY) {
      APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: Update calendar pin (bytesize=%d)", calendar_t->length);
      prv_decode_calendar(calendar_t->value->data, calendar_t->length);
      updated = true;
    }

    if (updated) {
      s_data.last_updated = time(NULL);
      prv_save_data();
    }

    // Update the user Timeline when a synced alarm/timer timestamp changes.
    //APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: alarm pin (%d) (%u != %u)", (int)s_alarm_pin, s_data.alarm_epoch, old_alarm);
    if (s_alarm_pin && s_data.alarm_epoch != old_alarm) {
      APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: Update alarm pin (%u => %u)", old_alarm, s_data.alarm_epoch);
      prv_sync_alarm_pin();
    }
    //APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: timer pin (%d) (%u != %u)", (int)s_timer_pin, s_data.timer_epoch, old_timer);
    if (s_timer_pin && s_data.timer_epoch != old_timer) {
      APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: Update timer pin (%u => %u)", old_timer, s_data.timer_epoch);
      prv_sync_timer_pin();
    }

    if (updated)
      return;
  }

  // Forward the dictionary to the watchface's settings handler (only one inbox
  // handler can be registered via the SDK, so we act as its dispatcher).
  if (s_forward) {
    s_forward(iter, context);
  }
}

// ---------------------------------------------------------------------------
// AppMessage event handlers (inbox dropped / outbox sent / outbox failed)
// ---------------------------------------------------------------------------

// Translate an AppMessageResult code into a short, readable name so the app log
// is easier to interpret. Error events pass an AppMessageResult reason code.
static const char *prv_result_name(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK:                            return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT:                  return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED:                 return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED:                 return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING:               return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS:                  return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY:                          return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW:               return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED:              return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED:   return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED:       return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY:                 return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED:                        return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR:                return "APP_MSG_INTERNAL_ERROR";
    default:                                    return "APP_MSG_INVALID_OTHER";
  }
}

// An incoming message was delivered by the phone, but the inbox buffer was full
// (or otherwise could not be processed in time), so the message was dropped.
static void prv_inbox_dropped_handler(AppMessageResult reason, void *context) {
  (void)context;
  APP_LOG(APP_LOG_LEVEL_WARNING, "AppMessage: inbox message dropped (%d, %s)",
          (int)reason, prv_result_name(reason));
}

// An outbound message was transmitted and acknowledged by the phone.
static void prv_outbox_sent_handler(DictionaryIterator *sent, void *context) {
  (void)sent;
  (void)context;
  APP_LOG(APP_LOG_LEVEL_INFO, "AppMessage: outbound message sent successfully");
}

// An outbound message was not accepted/transmitted by the phone.
static void prv_outbox_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  (void)failed;
  (void)context;
  APP_LOG(APP_LOG_LEVEL_WARNING, "AppMessage: outbound message failed (%d, %s)",
          (int)reason, prv_result_name(reason));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void alarm_calendar_sync_init(AppMessageInboxReceived forward) {
  s_forward = forward;
  prv_load_data();

  // Open AppMessage and register our inbox handler. Buffers are sized to match
  // the original settings traffic.
  app_message_open(768, 256);
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_register_inbox_dropped(prv_inbox_dropped_handler);
  app_message_register_outbox_sent(prv_outbox_sent_handler);
  app_message_register_outbox_failed(prv_outbox_failed_handler);
}

void alarm_calendar_sync_set_enabled(bool enabled) {
  s_enabled = enabled;
}

void alarm_calendar_sync_set_alarm_pin(bool enabled) {
  s_alarm_pin = enabled;
}

void alarm_calendar_sync_set_timer_pin(bool enabled) {
  s_timer_pin = enabled;
}

void alarm_calendar_sync_maybe_request_update(void) {
  if (!s_enabled || !connection_service_peek_pebble_app_connection()) {
    return;
  }

  time_t now = time(NULL);
  if ((now - s_data.last_updated) >= SYNC_DATA_FRESH_SECONDS) {
    APP_LOG(APP_LOG_LEVEL_INFO, "AlarmCalSync: data stale, requesting re-sync");
    prv_send_sync_request();
  }
}

uint32_t alarm_calendar_sync_get_alarm(void) {
  return s_data.alarm_epoch;
}

uint32_t alarm_calendar_sync_get_timer(void) {
  return s_data.timer_epoch;
}

CalendarEvent alarm_calendar_sync_get_event_at(int index) {
  if (index < 0 || index >= MAX_CALENDAR_EVENTS) {
    return (CalendarEvent){ 0, 0 };
  }
  return s_data.calendar[index];
}

int alarm_calendar_sync_get_event_count(void) {
  return s_data.event_count;
}

time_t alarm_calendar_sync_last_updated(void) {
  return s_data.last_updated;
}

// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
var customClay = require('./custom-clay');
var Themes = require('./themes');
var MessageKeys = require('message_keys');

var builtinThemes = Themes.fetchBuiltinThemes();

// Initialize Clay
var clay = new Clay(clayConfig, customClay, { userData: { builtinThemes: builtinThemes } });

// Intercept settings after they're saved to localStorage, but before they're sent to the watch.
// These are potentially long strings and are irrelevant to the watchface.
var oldGetSettings = clay.getSettings;
clay.getSettings = (response, convert) => {
  var ret = oldGetSettings(response, convert);

  for (var key of [MessageKeys.XCLAYUserThemes, MessageKeys.XCLAYActiveTheme]) {
    if (key in ret) {
      delete ret[key];
    }
  }

  return ret;
}


// ---------------------------------------------------------------------------
// Synced alarm/timer Timeline pins
// ---------------------------------------------------------------------------
// When the watch syncs a new alarm or timer timestamp (and the corresponding
// timeline-pin setting is enabled) it sends a PinCreate / PinDelete command.
// We maintain one user Timeline pin per type, keyed by a fixed id, so a create
// with a new timestamp replaces (removes + re-places) the previous pin and a
// delete removes it entirely. The pin is pushed via the Rebble timeline API.
//
// The message key ids come from package.json "messageKeys" (the compiler maps
// them to numeric ids); PinTarget is sent with value 0 = alarm, 1 = timer.
var MessageKeys = require('message_keys');

var PIN_TARGET_ALARM = 0;
var PIN_TARGET_TIMER = 1;

var TIMELINE_API_URL = 'https://timeline-api.getpebble.com/v1/user/pins/';

var PIN_CONFIG = [];
PIN_CONFIG[PIN_TARGET_ALARM] = {
  id: 'arc-view-synced-alarm',
  title: 'Phone alarm',
  tinyIcon: 'system://images/ALARM_CLOCK'
};
PIN_CONFIG[PIN_TARGET_TIMER] = {
  id: 'arc-view-synced-timer',
  title: 'Phone timer',
  tinyIcon: 'system://images/ALARM_CLOCK'
};

function timelineRequest(pin, type) {
  var url = TIMELINE_API_URL + pin.id;
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    console.log('Arc View timeline: ' + type + ' pin ' + pin.id +
                ' -> ' + this.status + ' ' + this.responseText);
  };
  xhr.onerror = function() {
    console.log('Arc View timeline: error ' + type + ' for pin ' + pin.id);
  };
  xhr.open(type, url);
  xhr.setRequestHeader('Content-Type', 'application/json');

  Pebble.getTimelineToken(function(token) {
    xhr.setRequestHeader('X-User-Token', token);
    //console.log('Arc View timeline: sending ' + JSON.stringify(pin) + ' to ' + url);
    xhr.send(JSON.stringify(pin));
  }, function(errorId, errorMessage) {
    console.log('Arc View timeline: could not get timeline token: ' +
                errorId + ' ' + errorMessage);
  });
}

function insertPin(target, epoch) {
  var cfg = PIN_CONFIG[target];
  if (!cfg) { return; }
  var timestr = new Date(epoch * 1000).toISOString();
  timelineRequest({
    id: cfg.id,
    time: timestr,
    layout: {
      type: 'genericPin',
      title: cfg.title,
      tinyIcon: cfg.tinyIcon
    },
    reminders: [{
      time: timestr,
      layout: {
        type: 'genericReminder',
        title: cfg.title,
        subtitle: "Time's up!",
        tinyIcon: cfg.tinyIcon
      },
    }]
  }, 'PUT');
}

function deletePin(target) {
  var cfg = PIN_CONFIG[target];
  if (!cfg) { return; }
  timelineRequest(
    { id: cfg.id }, 'DELETE');
}

Pebble.addEventListener('appmessage', function(e) {
  var payload = e.payload;
  //console.log('Arc View: appmessage received: ' + JSON.stringify(payload));
  if (!payload || ('PinTarget' in payload) === false) {
    return;
  }

  var target = payload['PinTarget'];

  if ('PinCreate' in payload) {
    console.log('Arc View: insert pin target ' + target + ' epoch ' + payload['PinCreate']);
    insertPin(target, payload['PinCreate']);
  } else if ('PinDelete' in payload) {
    console.log('Arc View: delete pin target ' + target);
    deletePin(target);
  }
});

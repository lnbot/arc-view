module.exports = function(minified) {
  var clayConfig = this;
  var _ = minified._;
  var $ = minified.$;
  var HTML = minified.HTML;

  ////////////////////
  // Config item requirement handlers

  function toggleIsSet(messageKey, defaultIfNotFound) {
    var invert = messageKey.startsWith("!");

    var item = clayConfig.getItemByMessageKey(messageKey.replace(/^!/, ''));
    if (item !== null) {
      console.log(`toggleIsSet: ${messageKey} = ${item.get()}, invert = ${invert}`);
      return invert ? !item.get() : item.get();
    }
    console.log(`toggleIsSet: ${messageKey} not found, invert = ${invert}`);
    return invert ? !defaultIfNotFound : defaultIfNotFound;
  }

  // Returns a event handler for Clay that turns off 'messageKey' if any
  // of the required toggles aren't in the right position
  function makeRequireToggleFunc(messageKey, requirements) {
    return function() {
      console.log(`ToggleRequire: ${messageKey} [${requirements}]`);
      if (requirements.every((req) => toggleIsSet(req, true))) {
        clayConfig.getItemByMessageKey(messageKey).enable();
      } else {
        clayConfig.getItemByMessageKey(messageKey).disable();
      }
    };
  }

  // Sets requirements for a config item to be active based on a list toggles
  function setItemRequirements(messageKey, requirements) {
    requirements.forEach((req) => {
      var item = clayConfig.getItemByMessageKey(req.replace(/^!/, ''));
      if (item !== null) {
        var func = makeRequireToggleFunc(messageKey, requirements)
        item.on('change', func);

        // Use the callback once to set the initial state
        func.call(item);
        console.log(`Set requirement for ${messageKey} <= ${req}`);
      }
    });
  }

  ////////////////////
  // Main Clay custom config

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    setItemRequirements("ShowWatchDialWindow", ["ForegroundShape", "OrbitComplications"]);
    setItemRequirements("TimelineAlarmPin", ["EnableAlarmCalendarSync"]);
    setItemRequirements("TimelineTimerPin", ["EnableAlarmCalendarSync"]);
    setItemRequirements("PosTop", ["!OrbitComplications"]);
    setItemRequirements("LogoText", ["EnableLogo"]);
  });
  
};
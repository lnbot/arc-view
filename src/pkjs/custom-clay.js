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
    if (item) {
      return invert ? !item.get() : item.get();
    }
    return invert ? !defaultIfNotFound : defaultIfNotFound;
  }

  // Returns a event handler for Clay that turns off 'messageKey' if any
  // of the required toggles aren't in the right position
  function makeRequireAllToggleFunc(messageKey, requirements) {
    return function() {
      if (requirements.every((req) => toggleIsSet(req, true))) {
        clayConfig.getItemByMessageKey(messageKey).enable();
      } else {
        clayConfig.getItemByMessageKey(messageKey).disable();
      }
    };
  }

  // Returns a event handler for Clay that turns off 'messageKey' if any
  // of the required toggles aren't in the right position
  function makeRequireAnyToggleFunc(messageKey, requirements) {
    return function() {
      if (requirements.some((req) => toggleIsSet(req, true))) {
        clayConfig.getItemByMessageKey(messageKey).enable();
      } else {
        clayConfig.getItemByMessageKey(messageKey).disable();
      }
    };
  }

  // Sets requirements for a config item to be active based on a list toggles
  function setItemRequirements(messageKey, requirements, testFunc) {
    var testFunc = testFunc ? testFunc : makeRequireAllToggleFunc;

    requirements.forEach((req) => {
      var item = clayConfig.getItemByMessageKey(req.replace(/^!/, ''));
      if (item) {
        var func = testFunc(messageKey, requirements)
        item.on('change', func);

        // Use the callback once to set the initial state
        func.call(item);
        console.log(`Set requirement for ${messageKey} <= ${req}`);
      }
    });
  }

  ////////////////////
  // Theme related functionality

  var Themes =
    (function() {
      const PreviousThemeKey = 'pt';
      const DomIdPrefix = 'theme-';
      const UserThemesItemId = 'XCLAYUserThemes';
      const UserActiveThemeItemId = 'XCLAYActiveTheme';
      var doc = null;
      var themeElement = null;
      var themeTemplate = null;
      var builtinThemes = { version: 0, themes: {} };
      var userThemes = { nextidx: 1, themes: {} };
      var previousTheme = null;
      var activeThemeKey = null;
      var currentThemeDirty = false;

      function isBuiltin(key) { return key in builtinThemes.themes; }
      function isReadOnly(key) { return isBuiltin(key) || key == PreviousThemeKey; }
      function emptyUserThemes() { return { nextidx: 1, themes: {} }; }

      function getConfigValueByMessageKey(messageKey) {
        var item = clayConfig.getItemByMessageKey(messageKey);
        if (!item)
          return null;
        return item.get();
      }

      function getConfigValueById(id) {
        var item = clayConfig.getItemById(id);
        if (!item)
          return null;
        return item.get();
      }

      function setConfigValueById(id, value) {
        var item = clayConfig.getItemById(id);
        if (item)
          item.set(value);
        return item;
      }

      function getNextUserKey() {
        if (!userThemes)
          userThemes = emptyUserThemes();

        var next = `u${userThemes.nextidx}`;
        userThemes.nextidx += 1;
        return next;
      }

      function setActiveTheme(key) {
        // Needs to be separate from the radio buttons because those are dynamically set up
        var item = setConfigValueById(UserActiveThemeItemId, key);
        if (item)
          activeThemeKey = key;
        else
          console.error(`setActiveTheme: Couldn't set active template ${key}`);
        return item !== null;
      }

      function forEachTheme(f) {
        for (var t of Object.keys(builtinThemes.themes)) {
          f(t, builtinThemes.themes[t]);
        }

        if (previousTheme !== null)
          f(PreviousThemeKey, previousTheme);

        if (userThemes) {
          for (var ut of Object.keys(userThemes.themes)) {
            f(ut, userThemes.themes[ut]);
          }
        }
      }

      function getThemeElement() {
        if (themeElement === null) {
          // var elements = doc.getElementsByClassName('component-radio');
          // console.log(`getThemeElement: document=${doc}, elements=${elements.length}`);
          // for (var e of elements) {
          //   if (e.children.length >= 1 && e.children[0].innerHTML == 'COLOUR THEME SELECT') {
          //     e.children[0].innerHTML = "TEST TEST";
          //     themeElement = e.getElementsByClassName('radio-group')[0];
          //     break;
          //   }
          // }
          var radioInput = doc.getElementById('ThemeSelectRadioGroup');
          console.log(`getThemeElement: document=${doc}, radioInput=${radioInput}`);
          if (radioInput) {
            themeElement = radioInput.parentElement.parentElement;
            console.log(`getThemeElement: themeElement class='${themeElement.classList}' == radio-group`);
          }
        }
        return themeElement;
      }

      function getThemeTemplate() {
        if (themeTemplate === null) {
          var root = getThemeElement();
          for (var e of root.children) {
            if (e.innerHTML.includes('%Template')) {
              themeTemplate = e.outerHTML;
              e.remove();
              break;
            }
          }
        }
        return themeTemplate;
      }

      function addThemeToPage(label, value) {
        var template = getThemeTemplate();
        template = template.replace("%TemplateLabel", label).replace("%TemplateValue", value).
          replace("ThemeSelectRadioGroup", `ThemeSelect-${value}`);
        var element = getThemeElement();

        const newTheme = doc.createElement('theme');
        newTheme.innerHTML = template.trim();
        var addElement = newTheme.children[0];
        addElement.id = `${DomIdPrefix}${value}`
        element.appendChild(addElement);
      }

      function removeThemeFromPage(key) {
        var idToRemove = `${DomIdPrefix}${key}`

        var toRemove = doc.getElementById(idToRemove);
        if (toRemove)
          toRemove.remove();
      }

      function saveIntoTheme(theme) {
        for (var item of clayConfig.getItemsByType('color')) {
          if (item.messageKey === null)
            console.log(`saveIntoTheme: MessageKey null!`);
          theme[item.messageKey] = item.get();
        }

        return theme;
      }

      function saveCurrentTheme(name, key) {
        // Saves the initial state of the current theme on opening settings
        var curr = { 'name': name, 'key': key };
        return saveIntoTheme(curr);
      }

      function copyTheme() {
        try {
          var currKey = activeThemeKey;
          if (!currKey) {
            console.error(`copyTheme: Active theme key not found`);
            return;
          }

          var newName = getConfigValueById('ThemeCopyDelete');
          var newKey = getNextUserKey();
          if (!newName)
            newName = `User config ${newKey.substring(1)}`;

          var newTheme = saveCurrentTheme(newName, newKey);
          userThemes.themes[newKey] = newTheme;
          addThemeToPage(newName, newKey);
          selectTheme(newKey);
          console.log(`copyTheme: New theme name=${newName} key=${newKey}`);
        } catch (e) {
          console.log(`copyTheme: Error ${e}`);
        }
      }

      function deleteTheme() {
        try {
          var currKey = activeThemeKey;
          if (!currKey)
            return;

          var newName = getConfigValueById('ThemeCopyDelete');
          if (!newName || newName.toLowerCase() != 'delete') {
            setConfigValueById('ThemeCopyDelete', "Type 'delete' here then click Delete again");
            return;
          }

          if (!(currKey in userThemes.themes)) {
            console.error(`deleteTheme: Trying to delete non-user theme '${currKey}'`);
            return;
          }

          delete userThemes.themes[currKey];
          removeThemeFromPage(currKey);
          selectTheme(PreviousThemeKey);
          setConfigValueById('ThemeCopyDelete', '');
        } catch (e) {
          console.log(`deleteTheme: Error ${e}`);
        }
      }

      function getTheme(key) {
        if (previousTheme.key == key)
          return previousTheme;
        if (key in builtinThemes.themes)
          return builtinThemes.themes[key];
        if (key in userThemes.themes)
          return userThemes.themes[key];
        console.error(`getTheme: No theme '${key}'`)
        return null;
      }

      function selectTheme(key) { setConfigValueById('ThemeSelect', key); }

      // Handler.  Should not be referenced directly
      function _displayTheme(key) {
        console.log(`displayTheme: Changing to theme ${key}`);
        var theme = getTheme(key);
        if (theme === null) {
          console.error(`displayTheme: Can't find theme key=${key}`);
          return;
        }

        var unusedKeys = new Set(Object.keys(theme).filter((k) => k.endsWith('Color')));

        // Copy all theme colors to the color config section
        for (var item of clayConfig.getItemsByType('color')) {
          if (item.messageKey in theme) {
            item.set(theme[item.messageKey]);
            unusedKeys.delete(item.messageKey);
          } else {
            console.info(`displayTheme: No messageKey '${item.messageKey}' in theme '${key}'`);
          }
        };

        if (unusedKeys.size > 0) {
          var totalKeys = Object.keys(theme).filter((k) => k.endsWith('Color')).size;
          console.log(`displayTheme: Unused message keys count=${unusedKeys.size}/${totalKeys} key=${key}`);
        }

        // Don't let the user click delete on a read only theme
        var delButton = clayConfig.getItemById('DeleteThemeButton');
        if (delButton) {
          if (isReadOnly(key))
            delButton.disable();
          else
            delButton.enable();
        }

        setActiveTheme(key);
        currentThemeDirty = false;
      }

      function flushActiveTheme() {
        var key = activeThemeKey;
        if (!key || isReadOnly(key)) {
          console.log(`flushTheme: '${key}' is read-only`);
          return;
        }

        var theme = getTheme(key);
        if (!theme)
          return;

        saveIntoTheme(theme);
      }

      function readUserThemes() {
        try {
          var rawstr = getConfigValueByMessageKey(UserThemesItemId)

          if (rawstr) {
            userThemes = JSON.parse(rawstr);
          } else {
            console.log(`readUserThemes: Empty user themes`);
            userThemes = emptyUserThemes();
          }
        } catch (e) {
          console.error(`readUserThemes: JSON.parse error '${e}'`);
          userThemes = emptyUserThemes();
        }
      }

      function writeUserThemes() {
        try {
          flushActiveTheme();
          var target = clayConfig.getItemById(UserThemesItemId)

          if (target) {
            var rawstr = JSON.stringify(userThemes);
            target.set(rawstr);
            console.log(`writeUserThemes: Saved ${rawstr.length} chars`);
          } else {
            console.log(`writeUserThemes: No target for writing themes`);
          }
        } catch (e) {
          console.error(`writeUserThemes: JSON.stringify error '${e}'`);
        }
      }

      function initThemes(rootdocument) {
        doc = rootdocument;

        // Need to hide the UI elements without making Clay ignore tthem
        doc.getElementById("UserThemesInput").parentElement.parentElement.style.display = "none";
        doc.getElementById("ActiveThemeInput").parentElement.parentElement.style.display = "none";

        builtinThemes = clayConfig.meta.userData.builtinThemes;
        for (var k of Object.keys(builtinThemes.themes)) {
          var theme = builtinThemes.themes[k];
          theme.name = `* ${theme.name}`;
        }

        console.log(`Themes: version ${builtinThemes.version} error='${builtinThemes.error}' meta='${builtinThemes.meta}' cached=${builtinThemes.cached}`);
        previousTheme = saveCurrentTheme('- Previous Theme -', PreviousThemeKey);
        readUserThemes();

        forEachTheme((key, theme) => {
          theme.key = key;
          addThemeToPage(theme.name, key);
        });

        for (var item of clayConfig.getItemsByType('color')) {
          item.on('change', () => {
            currentThemeDirty = true;
          });
        }

        // UserActiveThemeId should have been populated from the saved state
        activeThemeKey = getConfigValueById(UserActiveThemeItemId);
        activeThemeKey = activeThemeKey ? activeThemeKey : previousTheme.key;
        var radio = clayConfig.getItemById('ThemeSelect');
        radio.on('change', () => _displayTheme(radio.get()));
        radio.set(activeThemeKey);

        var delButton = clayConfig.getItemById('DeleteThemeButton');
        if (delButton) {
          delButton.on('click', deleteTheme);
        }

        var copyButton = clayConfig.getItemById('CopyThemeButton');
        if (copyButton) {
          copyButton.on('click', copyTheme);
        }

        var submitButtons = clayConfig.getItemsByType('submit');
        for (var i of submitButtons) {
          i.on('click', writeUserThemes);
        }
      }

      return {
        init: initThemes,
        forEach: forEachTheme,
        isBuiltin: isBuiltin,
        isReadOnly: isReadOnly,
        write: writeUserThemes,
      };
    })();

  ////////////////////
  // Main Clay custom config

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    try {
      setItemRequirements("ShowWatchDialWindow", ["ForegroundShape", "OrbitComplications"]);
      setItemRequirements("TimelineAlarmPin", ["EnableAlarmCalendarSync"]);
      setItemRequirements("TimelineTimerPin", ["EnableAlarmCalendarSync"]);
      setItemRequirements("PosTop", ["!OrbitComplications"]);
      setItemRequirements("LogoText", ["EnableLogo"]);
      setItemRequirements("DigitalHour", ["!BlankFaceMode", "QuietTimeBlankFace"], makeRequireAnyToggleFunc);
      setItemRequirements("QuietTimeBlankFace", ["BlankFaceMode"]);
      setItemRequirements("HideUnlitMinimizedTicks", ["ShowWatchDialWindow"]);

      Themes.init(document);

    } catch (e) {
      alert(`on(AFTER_BUILD): Error ${e}`);
    }
  });

  clayConfig.on(clayConfig.EVENTS.BEFORE_DESTROY, function() {
    console.log('clayConfig.on EVENT.BEFORE_DESTROY');
    Themes.write();
  });
};
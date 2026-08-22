const PebbleColors = {
  "MintGreen": "#AAFFAA",
  "Melon": "#FFAAAA",
  "ShockingPink": "#FF55FF",
  "Folly": "#FF0055",
  "SunsetOrange": "#FF5555",
  "ArmyGreen": "#555500",
  "DukeBlue": "#0000AA",
  "TiffanyBlue": "#00AAAA",
  "ScreaminGreen": "#55FF55",
  "PastelYellow": "#FFFFAA",
  "RichBrilliantLavender": "#FFAAFF",
  "BrightGreen": "#55FF00",
  "BrilliantRose": "#FF55AA",
  "CadetBlue": "#55AAAA",
  "RoseVale": "#AA5555",
  "FashionMagenta": "#FF00AA",
  "JaegerGreen": "#00AA55",
  "BabyBlueEyes": "#AAAAFF",
  "Purpureus": "#AA55AA",
  "ChromeYellow": "#FFAA00",
  "DarkGreen": "#005500",
  "Red": "#FF0000",
  "Liberty": "#5555AA",
  "LightGray": "#AAAAAA",
  "VividViolet": "#AA00FF",
  "Rajah": "#FFAA55",
  "Indigo": "#5500AA",
  "MayGreen": "#55AA55",
  "Icterine": "#FFFF55",
  "BulgarianRose": "#550000",
  "Orange": "#FF5500",
  "Green": "#00FF00",
  "WindsorTan": "#AA5500",
  "LavenderIndigo": "#AA55FF",
  "DarkGray": "#555555",
  "ElectricBlue": "#55FFFF",
  "BlueMoon": "#0055FF",
  "Cyan": "#00FFFF",
  "Black": "#000000",
  "MediumAquamarine": "#55FFAA",
  "DarkCandyAppleRed": "#AA0000",
  "Limerick": "#AAAA00",
  "CobaltBlue": "#0055AA",
  "Celeste": "#AAFFFF",
  "ElectricUltramarine": "#5500FF",
  "PictonBlue": "#55AAFF",
  "Inchworm": "#AAFF55",
  "Blue": "#0000FF",
  "VividCerulean": "#00AAFF",
  "Purple": "#AA00AA",
  "KellyGreen": "#55AA00",
  "Malachite": "#00FF55",
  "MidnightGreen": "#005555",
  "Yellow": "#FFFF00",
  "Magenta": "#FF00FF",
  "SpringBud": "#AAFF00",
  "JazzberryJam": "#AA0055",
  "VeryLightBlue": "#5555FF",
  "White": "#FFFFFF",
  "IslamicGreen": "#00AA00",
  "OxfordBlue": "#000055",
  "ImperialPurple": "#550055",
  "Brass": "#AAAA55",
  "MediumSpringGreen": "#00FFAA"
};

var reverse = null;

class ColorError extends Error {
  constructor(message, color) {
    super(message);
    this.name = "ColorError";
    this.color = color;
  }
}

function isRGB(color) { return /^#[0-9A-F]{6}$/.test(color); }

function reverseMap(obj) {
  var rev = {};

  Object.keys(obj).forEach((k) => {
    rev[obj[k]] = k;
  });
  return rev;
}

function lookupColorName(color) {
  if (color != isRGB(color)) {
    if (reverse === null) { reverse = ReverseMap(PebbleColors); }

    if (color in reverse)
      return reverse[color];
  }

  throw new ColorError("RGB color not found", color);
}

function colorFromName(name, defaultColor) {
  if (name !== null && name in PebbleColors)
    return PebbleColors[name];
  if (defaultColor === null)
    throw new ColorError("No color found with name", name);
  return defaultColor;
}

// Resolve a color name into a #RRGGBB value or returns the input
// if it's already in that format
function resolveColor(color, defaultColor) {
  if (color !== null) {
    if (isRGB(color)) {
      return color;
    }
    return colorFromName(color, defaultColor);
  }

  if (defaultColor === null)
    throw new ColorError("No color found with name", color);
  return defaultColor;
}

module.exports = {
  toName: lookupColorName,
  resolve: resolveColor,
  fromName: colorFromName,
};
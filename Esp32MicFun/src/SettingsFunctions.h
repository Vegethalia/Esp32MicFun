#pragma once

// Variables defined in Effects headers (included after this header in main.cpp)
extern int16_t* _pBuffAudio2Send;
extern uint32_t _LastCalcKeyPressed;

void UpdatePref(Prefs thePref) {
  switch (thePref) {
    case Prefs::PR_INTENSITY:
      _ThePrefs.putUShort(PREF_INTENSITY, _MAX_MILLIS);
      break;
    case Prefs::PR_STYLE:
      _ThePrefs.putUChar(PREF_STYLE, (uint8_t)_TheDrawStyle);
      break;
    case Prefs::PR_GROUPMINS:
      _ThePrefs.putUShort(PREF_CURRENT_BY_MINUTES, _AgrupaConsumsPerMinuts);
      break;
    case Prefs::PR_PIANOMODE:
      _ThePrefs.putBool(PREF_PIANOMODE, _pianoMode);
      break;
    case Prefs::PR_NIGHTMODE:
      _ThePrefs.putBool(PREF_NIGHTMODE, _NightMode);
      break;
    case Prefs::PR_CUSTOM_HUE:
      _ThePrefs.putInt(PREF_CUSTOM_HUE, _TheDesiredHue);
      break;
    case Prefs::PR_DAYLIGHT_SAVING:
      _ThePrefs.putBool(PREF_DAYLIGHT_SAVING, _DaylightSaving);
      break;
  }
}

void ProcessStyleChange(DRAW_STYLE oldStyle, DRAW_STYLE newStyle) {
  if (oldStyle == DRAW_STYLE::CALC_MODE) {
    _u8g2.setFont(u8g2_font_princess_tr);
    _StartedCalcMode = -1;
  } else if (oldStyle == DRAW_STYLE::HORIZ_FIRE) {
    if (_pBuffAudio2Send) {
      free(_pBuffAudio2Send);
      _pBuffAudio2Send = nullptr;
    }
  }
}

DRAW_STYLE ClampDrawStyle(int styleValue, DRAW_STYLE maxStyle = DRAW_STYLE::MAX_STYLE) {
  return (DRAW_STYLE)max(min(styleValue, (int)maxStyle), (int)DRAW_STYLE::BARS_WITH_TOP);
}

uint8_t ClampBinGrouping(int value) {
  return (uint8_t)max(min(value, 8), 1);
}

uint8_t GetMaxBarSpacingForGrouping(uint8_t grouping) {
  return grouping > 1 ? (grouping - 1) : 0;
}

namespace {
bool ApplyDrawStyleChange(DRAW_STYLE style, bool forceChange, bool persist) {
  bool allowStyleChange = _TheDrawStyle != DRAW_STYLE::CALC_MODE || forceChange;

  if (allowStyleChange && _TheDrawStyle != style) {
    ProcessStyleChange(_TheDrawStyle, style);
    _TheDrawStyle = style;
    if (style != DRAW_STYLE::CALC_MODE && persist) {
      UpdatePref(Prefs::PR_STYLE);
    } else {
      _LastCalcKeyPressed = millis();
    }

    if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
      _UpdateCurrentNow = true;
    }

    SendDebugMessage(Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str());
    return true;
  }

  if (allowStyleChange && persist && style != DRAW_STYLE::CALC_MODE) {
    UpdatePref(Prefs::PR_STYLE);
  }
  return false;
}
}  // namespace

bool ChangeDrawStyle(DRAW_STYLE style, bool forceChange = false, bool persist = true) {
  if (!_xQueStyleChange) {
    return ApplyDrawStyleChange(style, forceChange, persist);
  }
  DrawStyleChangeRequest req;
  req.style = style;
  req.forceChange = forceChange;
  req.persist = persist;
  return xQueueOverwrite(_xQueStyleChange, &req) == pdPASS;
}

void ProcessPendingStyleChanges() {
  if (!_xQueStyleChange) {
    return;
  }
  DrawStyleChangeRequest req;
  while (xQueueReceive(_xQueStyleChange, &req, 0) == pdPASS) {
    ApplyDrawStyleChange(req.style, req.forceChange, req.persist);
  }
}

void SetNightMode(bool nightOn, bool persistStyle = true) {
  _NightMode = nightOn;
  if (nightOn) {
    _MAX_MILLIS = NIGHT_MILLIS;
    ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true, persistStyle);
    _barAlterDraw = ALTERDRAW::ODD;
    _FadingWaveMode = false;  // no volem fer fading en aquest mode
    _barSpacing = 4;
    _binGrouping = 5;
    _ShazamSongs = false;  // no volem detectar cançons en mode nit
  } else if (!nightOn) {
    _MAX_MILLIS = DEFAULT_MILLIS;
    ChangeDrawStyle(DRAW_STYLE::VERT_FIRE, true, persistStyle);
    _ShazamSongs = true;  // volem detectar cançons en mode dia
  }
  FastLED.setBrightness(_MAX_MILLIS);
}

bool ApplyIntensitySetting(int intensity, bool persist = true) {
  uint16_t newIntensity = (uint16_t)max(min(intensity, 255), 0);
  if (_MAX_MILLIS == newIntensity) {
    if (persist) {
      UpdatePref(Prefs::PR_INTENSITY);
    }
    return false;
  }

  _MAX_MILLIS = newIntensity;
  FastLED.setBrightness(_MAX_MILLIS);
  if (persist) {
    UpdatePref(Prefs::PR_INTENSITY);
  }
  SendDebugMessage(Utils::string_format("Updated intensity=%d", _MAX_MILLIS).c_str());
  return true;
}

bool ApplyDesiredHueSetting(int hue, bool persist = true) {
  int16_t newHue = (int16_t)max(min(hue, 255), -1);
  if (_TheDesiredHue == newHue) {
    if (persist) {
      UpdatePref(Prefs::PR_CUSTOM_HUE);
    }
    return false;
  }

  _TheDesiredHue = newHue;
  _DesiredHueLastSet = millis();
  if (persist) {
    UpdatePref(Prefs::PR_CUSTOM_HUE);
  }
  SendDebugMessage(Utils::string_format("Updated Hue=%d", _TheDesiredHue).c_str());
  return true;
}

bool ApplyPianoModeSetting(bool enabled, bool persist = true) {
  if (_pianoMode == enabled) {
    if (persist) {
      UpdatePref(Prefs::PR_PIANOMODE);
    }
    return false;
  }

  _pianoMode = enabled;
  if (persist) {
    UpdatePref(Prefs::PR_PIANOMODE);
  }
  SendDebugMessage(Utils::string_format("Piano Mode %s!!", _pianoMode ? "ON" : "OFF").c_str());
  return true;
}

bool ApplyBinGroupingSetting(int value) {
  uint8_t newGrouping = ClampBinGrouping(value);
  bool changed = _binGrouping != newGrouping;

  _binGrouping = newGrouping;
  uint8_t maxSpacing = GetMaxBarSpacingForGrouping(_binGrouping);
  if (_barSpacing > maxSpacing) {
    _barSpacing = maxSpacing;
    changed = true;
  }

  if (changed) {
    SendDebugMessage(Utils::string_format("BinGrouping=%d", (int)_binGrouping).c_str());
  }
  return changed;
}

bool ApplyBarSpacingSetting(int value) {
  uint8_t maxSpacing = GetMaxBarSpacingForGrouping(_binGrouping);
  uint8_t newSpacing = (uint8_t)max(min(value, (int)maxSpacing), 0);
  if (_barSpacing == newSpacing) {
    return false;
  }

  _barSpacing = newSpacing;
  SendDebugMessage(Utils::string_format("BarSpacing=%d", (int)_barSpacing).c_str());
  return true;
}

bool ApplyBarAlterDrawSetting(int value) {
  ALTERDRAW newAlter = (ALTERDRAW)max(min(value, (int)ALTERDRAW::ALTERNATE), (int)ALTERDRAW::NO_ALTER);
  if (_barAlterDraw == newAlter) {
    return false;
  }

  _barAlterDraw = newAlter;
  SendDebugMessage(Utils::string_format("BarAlterDraw=%d", (int)_barAlterDraw).c_str());
  return true;
}

bool ApplyFadingWaveModeSetting(bool enabled) {
  if (_FadingWaveMode == enabled) {
    return false;
  }

  _FadingWaveMode = enabled;
  SendDebugMessage(Utils::string_format("Fading Wave Mode %s!!", _FadingWaveMode ? "ON" : "OFF").c_str());
  return true;
}

bool ApplyShazamModeSetting(bool enabled) {
  if (_ShazamSongs == enabled) {
    return false;
  }

  _ShazamSongs = enabled;
  if (enabled) {
    _ShazamActivationTime = millis();
    SendDebugMessage("Shazam Mode ON!!");
  } else {
    _DisplayAsapIndicator = false;
    SendDebugMessage("Shazam Mode OFF!!");
  }
  return true;
}

bool ApplyNightModeSetting(bool enabled, bool persist = true) {
  if (_NightMode == enabled) {
    if (persist) {
      UpdatePref(Prefs::PR_NIGHTMODE);
    }
    return false;
  }

  SetNightMode(enabled, persist);
  if (persist) {
    UpdatePref(Prefs::PR_NIGHTMODE);
  }
  SendDebugMessage(Utils::string_format("Updated Nightmode=%d", (int)enabled).c_str());
  return true;
}

void TriggerShazamRecognizeAsap() {
  ApplyShazamModeSetting(true);
  SendDebugMessage("Shazam Mode ON (ASAP)!!");
  _DisplayAsapIndicator = true;
  _AsapDetectionTime = millis();
  _ThePubSub.publish(TOPIC_RECOGNIZE_ASAP, "1", false);
}

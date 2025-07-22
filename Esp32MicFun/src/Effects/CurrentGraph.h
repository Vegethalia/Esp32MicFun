
void DrawCurrentGraph(MsgAudio2Draw& mad) {
  uint8_t maxV = GetMapMaxPixels();
  uint8_t everyKwh = GetPixelsPerKwh(maxV);
  static bool published = false;
  static uint8_t _incBoleta = 0;

  if (!published) {
    SendDebugMessage(Utils::string_format("maxV=%d, every=%d", maxV, everyKwh).c_str());
    published = true;
  }

  // Calculate max BASS power among first bars
  // _1stBarValue = 0;
  // for (uint16_t i = 2; i < THE_PANEL_WIDTH / 5; i++) {
  //     auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
  //     value = map(value, MIN_FFT_DB, MAX_FFT_DB, 5, 200);
  //     if (_1stBarValue < value) {
  //         _1stBarValue = value;
  //     }
  // }

  // pintem els eixos
  // Eix horitzontal

  CHSV eix = CHSV(HSVHue::HUE_BLUE, 255, 64);
  CHSV eixHour = CHSV(HSVHue::HUE_GREEN, 255, 64);
  CHSV kw = CHSV(HSVHue::HUE_PURPLE, 255, 80);

  for (uint8_t i = 0; i < THE_PANEL_WIDTH; i++) {
    _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1)] = eix;
  }
  // a cada canvi d'hora pintarem un pixel verd
  if (_pLectures) {
    uint32_t actHour = _pLectures[0].horaConsum / 3600;
    for (uint8_t i = 0; i < THE_PANEL_WIDTH; i++) {
      if (_pLectures[i].horaConsum > 0 && _pLectures[i].horaConsum / 3600 != actHour) {
        actHour = _pLectures[i].horaConsum / 3600;
        _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1)] = eixHour;
      }
    }
  }
  // Eix vertical
  // for (uint8_t j = 0; j < maxV; j++) {
  //     _TheLeds[_TheMapping.XY(0, THE_PANEL_HEIGHT - 2 - j)] = eix;
  // }
  // if ((_TheFrameNumber % 8) == 0) {
  //     _incBoleta = (_incBoleta + 1) % 4;
  // }
  if (_1stBarValue > 175) {
    _incBoleta = (_incBoleta + 1) % 4;
  }
  for (uint8_t j = everyKwh; j < maxV; j += everyKwh) {
    if (j >= maxV) {
      break;  // la línia de més amunt no la pintem
    }

    for (uint8_t i = 0 + _incBoleta; i < THE_PANEL_WIDTH; i += 4) {
      _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1 - j)] += kw;
    }
  }
  // i ara pintem les barres
  uint8_t previousBarValue = 0;
  if (_pLectures) {
    long mapValue = 0;
    uint8_t maxValue = 0;
    uint8_t intensity = max((int)100, (int)_1stBarValue);
    int intensityBar;
    for (uint8_t i = 0; i < THE_PANEL_WIDTH; i++) {
      mapValue = map(_pLectures[i].valorEnLeds, 0, _MapMaxWhToPixels, 0, maxV);

      // pintem "barres"
      // intensityBar = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
      // intensityBar = map(intensityBar, MIN_FFT_DB, MAX_FFT_DB + 5, 0, 140);
      intensityBar = map(mad.pDBs[i], 0, 255, 0, 140);
      for (uint8_t j = 1; j < mapValue; j++) {
        _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1 - j)] += CHSV(HSVHue::HUE_AQUA, 255, (uint8_t)intensityBar);
      }

      // pintem "top pixel"
      _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1 - mapValue)] += CHSV(HSVHue::HUE_YELLOW, 255, intensity);
      // _TheLeds[_TheMapping.XY(i + 1, THE_PANEL_HEIGHT - 2 - mapValue)] += CHSV(HSVHue::HUE_PINK, 255, 20 + (mapValue * 11));
      //
      if (mapValue > maxValue) {
        maxValue = mapValue;
      }
      if (i > 0) {  // pintem la línia que uneix les barres
        if (mapValue > previousBarValue) {
          for (uint8_t j = previousBarValue + 1; j < mapValue; j++) {
            _TheLeds[_TheMapping.XY(i - 1, THE_PANEL_HEIGHT - 1 - j)] += CHSV(HSVHue::HUE_YELLOW, 255, intensity);
          }
        } else if (mapValue < previousBarValue) {
          for (uint8_t j = mapValue + 1; j < previousBarValue; j++) {
            _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1 - j)] += CHSV(HSVHue::HUE_YELLOW, 255, intensity);
          }
        }
      }
      previousBarValue = mapValue;
    }
    // pintem el led que es va movent per sobre la linia de valors d'electricitat
    // uint8_t fiufiu = (uint8_t)(_TheFrameNumber % (THE_PANEL_WIDTH - 1));
    // mapValue = map(_pLectures[fiufiu].valorEnLeds, 0, _MapMaxWhToPixels, 0, maxV);
    // _TheLeds[_TheMapping.XY(fiufiu + 1, THE_PANEL_HEIGHT - 2 - mapValue)] += CHSV(HSVHue::HUE_YELLOW, 255, 200);
#if defined(PANEL_SIZE_96x54)
    if (maxValue < 20 && _MapMaxWhToPixels > 1500) {
      _MapMaxWhToPixels -= 500;
    } else if (maxValue > 30 && _MapMaxWhToPixels < 6000) {
      _MapMaxWhToPixels += 500;
    }
#else
    if (maxValue < 14 && _MapMaxWhToPixels > 1500) {
      _MapMaxWhToPixels -= 500;
    } else if (maxValue >= 18 && _MapMaxWhToPixels < 6000) {
      _MapMaxWhToPixels += 500;
    }
#endif
  }
}

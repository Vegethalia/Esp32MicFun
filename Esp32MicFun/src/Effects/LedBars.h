
void DrawLedBars(MsgAudio2Draw& mad) {
  const auto numItems = _TheMapping.GetWidth();
  uint8_t maxBassValue = 0;
  int16_t value = 0;
#if defined(PANEL_SIZE_64x32)
  uint8_t maxHeight = min(16, BAR_HEIGHT);  // 18
#elif defined(PANEL_SIZE_96x54)
  uint8_t maxHeight = min(24, BAR_HEIGHT);  // 26
#endif
  uint16_t maxHeightScaled = (maxHeight * 10) + 9;
  // static uint8_t __binGrouping = 4;  // group bins every this number
  static ALTERDRAW __alterDraw = ALTERDRAW::ODD;

  if (_TheLastKey != GEN_KEY_PRESS::KEY_NONE) {
    if (_TheLastKey == GEN_KEY_PRESS::KEY_INC1) {
      __alterDraw = (((uint8_t)__alterDraw < 2) ? (ALTERDRAW)(__alterDraw + 1) : ALTERDRAW::NO_ALTER);
    } else if (_TheLastKey == GEN_KEY_PRESS::KEY_DEC1) {
      __alterDraw = (((uint8_t)__alterDraw > 0) ? (ALTERDRAW)(__alterDraw - 1) : ALTERDRAW::ALTERNATE);
    } else if (_TheLastKey == GEN_KEY_PRESS::KEY_DEC2 && _binGrouping > 1) {
      _binGrouping--;
    } else if (_TheLastKey == GEN_KEY_PRESS::KEY_INC2 && _binGrouping < 8) {
      _binGrouping++;
    }
    SendDebugMessage(Utils::string_format("BinGrouping=%d", (int)_binGrouping).c_str());

    _TheLastKey = GEN_KEY_PRESS::KEY_NONE;
  }

  assert(BAR_HEIGHT > 1);
  // uint8_t minBoostBin = (uint8_t)(numItems * 0.33); // the first 13 bars in 33 width panel
  // constexpr float maxTrebleBoost = 30.0;
  // constexpr float minBassBoost = 1.0;
  // float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)numItems);

  // FastLED.clear();
  //  DrawImage();
  //_ThePanel.SetBaseHue();
  if (_TheDesiredHue < 0) {
    _ThePanel.SetBaseHue(HSVHue::HUE_BLUE);
  } else {
    _ThePanel.SetBaseHue((uint8_t)_TheDesiredHue);
  }

  for (uint16_t i = 0; i < numItems; i += _binGrouping) {
    // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
    //     auto boost = 1.0f + (i * freqBoost);
    //     value = (int)(value * boost);
    // }

    // value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
    // value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, maxHeightScaled); //

    if (_binGrouping > 1) {
      value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
      value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, maxHeightScaled);
      int32_t avg = value;
      int32_t max = value;
      for (uint8_t j = 1; j < _binGrouping; j++) {
        value = constrain(mad.pFftMag[i + j], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, maxHeightScaled);  //
        avg += value;
        if (value > max) {
          max = value;
        }
      }
      value = avg / _binGrouping;
      for (uint8_t j = 0; j < _binGrouping - _barSpacing; j++) {
        // _ThePanel.DrawBar(i + j - __binGrouping, max, false, __alterDraw);  // j%2==0?false:true);
        _ThePanel.DrawBar(i + j, max, false, __alterDraw);  // j%2==0?false:true);
      }
      // i += __binGrouping;
    } else {
      value = map(mad.pDBs[i], 0, 255, 0, maxHeightScaled);  //
      _ThePanel.DrawBar(i, value);
    }
  }
  //_ThePanel.IncBaseHue();
}

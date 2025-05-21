
void DrawWave(MsgAudio2Draw& mad) {
  struct WavePoint {
    uint16_t ledIndex;
    CHSV thePixel;
  };
  static WavePoint _WaveBuffer[MAX_FADING_WAVES][THE_PANEL_WIDTH];  // circular buffer of waves
  static uint8_t _LasttWaveIndex = 0;                               // index of the circular buffer, apunta a la wave que toca pintar
  static int16_t _OffsetMv = -5;                                    // adjust wave scale so it does not appears to flat or clipping
  static auto _lastIncrease = millis();
  static uint16_t _maxValueVeryHi = 0;  // max value drawn since _lastIncrease
  static uint16_t _maxValueHi = 0;      // max value drawn since _lastIncrease

  uint8_t height = _TheMapping.GetHeight() - 1;
  uint16_t width = _TheMapping.GetWidth();
  uint16_t i;
  int16_t value;
  uint8_t MAX_SCALE = 7;  // 8;

  // if(_numFrames % 2 == 0) {
  //   return;
  // }

  if (WITH_MEMS_MIC) {  // Escalem la ona a "mic analògic 9814" (historical reasons... S'HA DE REFER!!!)
    int16_t valueOrig;
    int16_t scale = MAX_SCALE + _OffsetMv;  // max value of _OffsetMv=(-9)
    for (i = 0; i < mad.audioLenInSamples; i++) {
      valueOrig = mad.pAudio[i];  // dividim per 2 per que amb audio alt l'ona és massa gran
      // valueOrig = mad.pAudio[i] / 2;  // dividim per 2 per que amb audio alt l'ona és massa gran
      //  if (valueOrig < INT16_MIN / scale) {
      //      valueOrig = INT16_MIN / scale;
      //  } else if (valueOrig > INT16_MAX / scale) {
      //      valueOrig = INT16_MAX / scale;
      //  }
      if (valueOrig > INT16_MAX / scale) {
        valueOrig = INT16_MAX / scale;
      }

      mad.pAudio[i] = map(valueOrig, INT16_MIN / scale, INT16_MAX / scale, INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
    }
  }

  // busquem el pass per "0" després de la muntanya més gran
  uint16_t pas0 = 0;
  uint16_t maxAmp = INPUT_0_VALUE;
  uint16_t iMaxAmp = 0;
  int maxSearch = (int)(width * 0.5f);
  for (i = 0; i < maxSearch; i++) {
    if (mad.pAudio[i] > maxAmp) {
      maxAmp = mad.pAudio[i];
      iMaxAmp = i;
    }
  }  // ja tenim l'index del pic de la muntanya mes gran. Ara busquem a on creuem per 0
  maxSearch = (int)(width * 1.5f);
  pas0 = iMaxAmp;
  for (i = iMaxAmp; i < maxSearch; i++) {
    if (mad.pAudio[i] <= INPUT_0_VALUE) {
      pas0 = i;
      break;
    }
  }

  CHSV myValue;
  //, movingPoint;
  // movingPoint.setHSV(HSVHue::HUE_PURPLE, 128, 70);
  myValue.setHSV(HSVHue::HUE_PURPLE, 255, 70);

  uint8_t numFadingWaves = 1;
  //    if (_TheDrawStyle != DRAW_STYLE::BARS_WITH_TOP) {
  // numFadingWaves = 2;
  myValue.setHSV(HSVHue::HUE_AQUA, 128, 70);
  //    }
  int16_t numValuesHi = 0;
  int16_t numValuesVeryHi = 0;
  for (i = 0; i < width; i += _WaveDrawEvery) {
    // #if defined(PANEL_SIZE_96x48)
    //     value = mad.pAudio[pas0 + i];
    //     // value += mad.pAudio[pas0 + (i * 3) + 2];
    //     // value = constrain(value / 3, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE - _OffsetMv), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE - _OffsetMv));
    //     // value = mad.pAudio[pas0 + i];
    //     // value = map(value / 2, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE - _OffsetMv), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE - _OffsetMv), 0, height);
    //     value = map(value, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE), 0, height);
    //     // if (value > height / 2) {
    //     //     sumValues += value;
    //     //     numValues++;
    //     // };
    // #elif
    value = mad.pAudio[pas0 + (i * 2)];
    value += mad.pAudio[pas0 + (i * 2) + 1];
    // value += mad.pAudio[pas0 + (i * 3) + 2];
    // value = constrain(value / 3, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE - _OffsetMv), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE - _OffsetMv));
    // value = mad.pAudio[pas0 + i];
    // value = map(value / 2, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE - _OffsetMv), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE - _OffsetMv), 0, height);
    value = map(value / 2, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE), 0, height);
    // if (value > height / 2) {
    //     sumValues += value;
    //     numValues++;
    // };
    // #endif
    if (value >= height - 4) {
      numValuesVeryHi++;
    } else if (value >= height - 8) {
      numValuesHi++;
    }

    // value = min((int16_t)max(value, (int16_t)0), (int16_t)height);
    _WaveBuffer[_LasttWaveIndex][i].ledIndex = _TheMapping.XY(i, value);
    _WaveBuffer[_LasttWaveIndex][i].thePixel = myValue;
    // if (_TheFrameNumber % width == i) {
    //     _WaveBuffer[_LasttWaveIndex][i].thePixel = movingPoint;
    // }
    for (byte j = 1; j < _WaveDrawEvery; j++) {
      _WaveBuffer[_LasttWaveIndex][i + j].ledIndex = _TheMapping.XY(i + j, value);
      _WaveBuffer[_LasttWaveIndex][i + j].thePixel.v = 0;
    }
  }
  if (numValuesHi > _maxValueHi) {
    _maxValueHi = numValuesHi;
  }
  if (numValuesVeryHi > _maxValueVeryHi) {
    _maxValueVeryHi = numValuesVeryHi;
  }
  // sumValues = sumValues / numValues;
  // if (sumValues > _maxValue) {
  //     _maxValue = sumValues;
  // }

  if ((millis() - _lastIncrease) > (15 * 1000)) {
    if (_maxValueVeryHi > (THE_PANEL_WIDTH / 6) && _OffsetMv > (-(MAX_SCALE - 1))) {
      _OffsetMv = _OffsetMv - ((_OffsetMv > (-5)) ? 2 : 1);
      _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("DEC WaveScale=%d, ValuesHi=%d ValuesVeryHi=%d", MAX_SCALE + _OffsetMv, _maxValueHi, _maxValueVeryHi).c_str());
    } else if (_maxValueHi < (THE_PANEL_WIDTH / 5) && _OffsetMv < (MAX_SCALE - 5)) {
      _OffsetMv++;
      _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("INC WaveScale=%d, ValuesHi=%d ValuesVeryHi=%d", MAX_SCALE + _OffsetMv, _maxValueHi, _maxValueVeryHi).c_str());
    }
    // else {
    //     _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("STAY --> Offset=%d ValuesHi=%d ValuesVeryHi=%d", _OffsetMv, _maxValueHi, _maxValueVeryHi).c_str());
    // }

    _maxValueVeryHi = 0;
    _maxValueHi = 0;
    _lastIncrease = millis();
  }

  if (numFadingWaves > 1) {  // fem desapareixer la 1era wave, que serà la última ara
    // pintem les n waves anteriors
    for (uint8_t numWave = 1; numWave < numFadingWaves; numWave++) {
      uint8_t currentWaveIndex = (_LasttWaveIndex + numWave) % numFadingWaves;
      for (i = 0; i < width; i++) {
        CHSV currColor = _WaveBuffer[currentWaveIndex][i].thePixel;
        // currColor.v = currColor.v / (numWave + 1);
        // currColor.s = currColor.s / 2;
        currColor.h = HSVHue::HUE_GREEN;
        // currColor.h + (15 * numWave);
        // currColor.v = currColor.v / (numWave + 1);
        _TheLeds[_WaveBuffer[currentWaveIndex][i].ledIndex] = currColor;
      }
    }
  }
  // ara pintem l'última (o l'única)
  for (i = 0; i < width; i++) {
    _TheLeds[_WaveBuffer[_LasttWaveIndex][i].ledIndex] = _WaveBuffer[_LasttWaveIndex][i].thePixel;
  }

  if (numFadingWaves > 1) {  // fem desapareixer la 1era wave, que serà la última ara
    _LasttWaveIndex = (_LasttWaveIndex + 1) % numFadingWaves;
  }

  //_ThePanel.IncBaseHue();
}

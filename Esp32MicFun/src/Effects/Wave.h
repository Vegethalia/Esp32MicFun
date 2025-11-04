#define ANAL_CLOCK_RADIUS ((THE_PANEL_HEIGHT / 2) - 1)

void DrawWave(MsgAudio2Draw& mad) {
  struct WavePoint {
    uint16_t ledIndex;
    // CHSV thePixel;
  };
  // static WavePoint _WaveBuffer[MAX_FADING_WAVES][THE_PANEL_WIDTH];  // circular buffer of waves
  static uint8_t _LasttWaveIndex = 0;  // index of the circular buffer, apunta a la wave que toca pintar
  static int16_t _OffsetMv = -5;       // adjust wave scale so it does not appears to flat or clipping
  static auto _lastIncrease = millis();
  static uint16_t _maxValueVeryHi = 0;                // max value drawn since _lastIncrease
  static uint16_t _maxValueHi = 0;                    // max value drawn since _lastIncrease
  static int16_t _previousWave[THE_PANEL_WIDTH * 2];  // ens guardem les 2 últimes ones per a fer un promig
  static std::vector<std::vector<WavePoint>> _WaveBuffer;

  if (_TheDrawStyle == DRAW_STYLE::VERT_FIRE) {
    _FadingWaveMode = false;  // no volem fer fading en aquest mode
  }

  if (_FadingWaveMode) {
    if (_WaveBuffer.size() != MAX_FADING_WAVES) {
      _WaveBuffer.resize(MAX_FADING_WAVES);
      for (uint8_t i = 0; i < MAX_FADING_WAVES; i++) {
        _WaveBuffer[i].resize(THE_PANEL_WIDTH);
      }
    }
  } else {
    _WaveBuffer.resize(1);
    _WaveBuffer[0].resize(THE_PANEL_WIDTH);
  }

  uint8_t height = _TheMapping.GetHeight() - 2;
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
  int maxSearch = 32;  //(int)(width * 0.5f);
  for (i = 0; i < maxSearch; i++) {
    if (mad.pAudio[i] > maxAmp) {
      maxAmp = mad.pAudio[i];
      iMaxAmp = i;
    }
  }  // ja tenim l'index del pic de la muntanya mes gran. Ara busquem a on creuem per 0
  // maxSearch = (int)(width * 1.5f);
  pas0 = iMaxAmp;
  for (i = iMaxAmp; i < maxSearch; i++) {
    if (mad.pAudio[i] <= INPUT_0_VALUE) {
      pas0 = i;
      break;
    }
  }

  CHSV myValue, analClockValue;
  //, movingPoint;
  // movingPoint.setHSV(HSVHue::HUE_PURPLE, 128, 70);
  myValue.setHSV(HSVHue::HUE_PURPLE, 255, 70);
  analClockValue = myValue;
  analClockValue.s -= 32;
  analClockValue.v -= 5;

  uint8_t numFadingWaves = _WaveBuffer.size();  // si es canvia això, caldrà modificar el #define MAX_FADING_WAVES 1
  //    if (_TheDrawStyle != DRAW_STYLE::BARS_WITH_TOP) {
  // numFadingWaves = 2;
  myValue.setHSV(HSVHue::HUE_AQUA, 128, 70);

  //    }
  int16_t numValuesHi = 0;
  int16_t numValuesVeryHi = 0;
  for (i = 0; i < width; i += _WaveDrawEvery) {
    value = mad.pAudio[pas0 + (i * 2)];
    value += mad.pAudio[pas0 + (i * 2) + 1];
    value = map(value / 2, INPUT_0_VALUE - (VOLTATGE_DRAW_RANGE), INPUT_0_VALUE + (VOLTATGE_DRAW_RANGE), 0, height);

    if (!_FadingWaveMode) {
      int16_t newValue = value;
      value = (_previousWave[i] + _previousWave[i + width] + value) / 3;
      _previousWave[i] = _previousWave[i + width];
      _previousWave[i + width] = newValue;
    }
    if (value >= height - 4) {
      numValuesVeryHi++;
    } else if (value >= height - 8) {
      numValuesHi++;
    }

    _WaveBuffer[_LasttWaveIndex][i].ledIndex = _TheMapping.XY(i, value);
    for (byte j = 1; j < _WaveDrawEvery; j++) {
      _WaveBuffer[_LasttWaveIndex][i + j].ledIndex = _TheMapping.XY(i + j, value);
    }
  }
  if (numValuesHi > _maxValueHi) {
    _maxValueHi = numValuesHi;
  }
  if (numValuesVeryHi > _maxValueVeryHi) {
    _maxValueVeryHi = numValuesVeryHi;
  }

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

  if (!_FadingWaveMode) {
    // ara pintem l'única wave
    if (_TheDrawStyle == DRAW_STYLE::ANALOG_CLOCK) {  // pintem el centre amb un color diferent
      for (i = 0; i < (width / 2) - ANAL_CLOCK_RADIUS; i++) {
        _TheLeds[_WaveBuffer[_LasttWaveIndex][i].ledIndex] = myValue;  // _WaveBuffer[_LasttWaveIndex][i].thePixel;
      }
      for (i = (width / 2) - ANAL_CLOCK_RADIUS; i < (width / 2) + ANAL_CLOCK_RADIUS; i++) {
        _TheLeds[_WaveBuffer[_LasttWaveIndex][i].ledIndex] = analClockValue;  // _WaveBuffer[_LasttWaveIndex][i].thePixel;
      }
      for (i = (width / 2) + ANAL_CLOCK_RADIUS; i < width; i++) {
        _TheLeds[_WaveBuffer[_LasttWaveIndex][i].ledIndex] = myValue;  // _WaveBuffer[_LasttWaveIndex][i].thePixel;
      }
    } else {
      for (i = 0; i < width; i++) {
        _TheLeds[_WaveBuffer[_LasttWaveIndex][i].ledIndex] = myValue;  // _WaveBuffer[_LasttWaveIndex][i].thePixel;
      }
    }
  } else {
    CHSV myValue2 = myValue;
    myValue2.v = 20;
    myValue2.s = 0;
    // myValue2.h -= 10;
    CRGB myValueRGB = myValue2;
    uint8_t basicLuma = myValueRGB.getLuma() - 1;
    // myValue2.v = 5;
    for (uint8_t numWave = 1; numWave < numFadingWaves; numWave++) {
      uint8_t currentWaveIndex = (_LasttWaveIndex + numWave) % numFadingWaves;
      for (i = 0; i < width; i++) {
        uint16_t ledIndex = _WaveBuffer[currentWaveIndex][i].ledIndex;

        _TheLeds[ledIndex] = myValue2;  // _WaveBuffer[currentWaveIndex][i].thePixel;

        // if (_TheLeds[ledIndex].getLuma() < basicLuma) {
        //   _TheLeds[ledIndex] = myValue2;
        // } else {
        //   _TheLeds[ledIndex] += myValue2;  // _WaveBuffer[currentWaveIndex][i].thePixel;
        // }
      }
      myValue2.v += 3;  //= 5;
      myValue2.s += 8;
      // myValue2.h += 20;
    }
    // ara pintem l'última
    myValue.v = 80;
    myValue.s = 160;
    for (i = 0; i < width; i++) {
      _TheLeds[_WaveBuffer[_LasttWaveIndex][i].ledIndex] = myValue;
    }
  }

  if (numFadingWaves > 1) {  // fem desapareixer la 1era wave, que serà la última ara
    _LasttWaveIndex = (_LasttWaveIndex + 1) % numFadingWaves;
  }
}

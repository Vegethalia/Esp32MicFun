
#define CLOCK_HORIZ_PIXELS 41
#define CLOCK_VERT_PIXELS 9 // 7

void DrawHorizSpectrogram(MsgAudio2Draw& mad)
{
    int16_t value = 0;

    //_TheLeds.fill_solid(CRGB(1,1,1));
    //   FastLED.clear();
    assert(BAR_HEIGHT > 1);
    const auto numItems = _TheMapping.GetHeight();
    uint8_t values[numItems];

    for (uint16_t i = 0; i < numItems; i++) {
        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, 255);
        values[i] = (uint8_t)value;
    }
    _ThePanel.PushBar(values);

    // FastLED.show();
}

uint8_t _1stBarValue = 128;
void DrawVertSpectrogram(MsgAudio2Draw& mad)
{
    int16_t value = 0;
    // static int lastFrame = 0;

    // if((millis()-lastFrame)<10) {
    //     return;
    // }

    //_TheLeds.fill_solid(CRGB(1,1,1));
    //   FastLED.clear();
    static int8_t incValue = +1;
    static int8_t actualInc = 0;
    static const int8_t MAX_INC = 32;

    if (_numFrames % 4 == 0) {
        actualInc += incValue;
        if (actualInc >= MAX_INC) {
            incValue = -1;
        } else if (actualInc == 0) {
            incValue = +1;
        }
    }

    // actualInc = random8(32);

    if (_TheDesiredHue < 0) {
        _ThePanel.SetBaseHue((HSVHue::HUE_ORANGE - (MAX_INC / 3)) + actualInc);
    } else {
        _ThePanel.SetBaseHue((_TheDesiredHue - (MAX_INC / 3)) + actualInc);
    }
    //_ThePanel.IncBaseHue();

    assert(THE_PANEL_WIDTH > 1);
    const auto numItems = _TheMapping.GetWidth();
    uint8_t values[numItems];
    memset8(values, 0, sizeof(values));

    // uint8_t minBoostBin = (uint8_t)(numItems * 0.33); // the first 13 bars in 33 width panel
    // constexpr float maxTrebleBoost = 5.0;
    // constexpr float minBassBoost = 1.0;
    // float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)numItems);
    _1stBarValue = 0;
    for (uint16_t i = 0; i < _TheMapping.GetWidth(); i++) {
        // int8_t decPower = 0;
        // if (i < 22) {
        //     decPower = 16;
        // } else if (i < 40) {
        //     decPower = 10;
        // } else if (i > (THE_PANEL_WIDTH - 10)) {
        //     decPower = (-10);
        // }

        // if (i > (THE_PANEL_WIDTH - 20)) {
        //     value = (int16_t)((float)value * (1.25 + (i / 10.0)));
        // }
        value = constrain(mad.pFftMag[i], (int)MIN_FFT_DB, MAX_FFT_DB);
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        // value = constrain(mad.pFftMag[i], MIN_FFT_DB-5, MAX_FFT_DB);
        value = map(value, (int)MIN_FFT_DB, MAX_FFT_DB, 0, 255);
        values[i] = (uint8_t)value;
        if (i <= 5 && _1stBarValue < value) {
            _1stBarValue = value;
        }
    }
    if (_WithClock) {
        _ThePanel.PushLine(values, THE_PANEL_WIDTH - CLOCK_HORIZ_PIXELS, CLOCK_VERT_PIXELS - 1);
        //_ThePanel.PushLine(values, 6, 12);
    } else {
        _ThePanel.PushLine(values);
    }

    // FastLED.show();
    // lastFrame = millis();
}

void DrawLedBars(MsgAudio2Draw& mad)
{
    const auto numItems = _TheMapping.GetWidth();
    uint8_t maxBassValue = 0;
    int16_t value = 0;
    uint8_t maxHeight = min(18, BAR_HEIGHT);
    uint8_t maxHeightScaled = (maxHeight * 10) + 9;

    assert(BAR_HEIGHT > 1);
    uint8_t minBoostBin = (uint8_t)(numItems * 0.33); // the first 13 bars in 33 width panel
    constexpr float maxTrebleBoost = 30.0;
    constexpr float minBassBoost = 1.0;
    float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)numItems);

    // FastLED.clear();
    //  DrawImage();
    //_ThePanel.SetBaseHue();
    if (_TheDesiredHue < 0) {
        _ThePanel.SetBaseHue(HSVHue::HUE_BLUE);
    } else {
        _ThePanel.SetBaseHue((HSVHue)_TheDesiredHue);
    }
    for (uint16_t i = 0; i < numItems; i++) {
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, maxHeightScaled); // fins a 89 // 169

        _ThePanel.DrawBar(i, value, 200);
        if (i > 3 && i < (THE_PANEL_WIDTH / 5) && _1stBarValue < value) {
            _1stBarValue = value;
        }
    }
    _1stBarValue = _1stBarValue / 10;
    /*    for (uint16_t i = 0; i < maxIndex; i++) {
            uint8_t bin = binByPow[i].binNum;
            if(bin==0) {
                continue;
            }
            // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
            //     auto boost = 1.0f + (bin * freqBoost);
            //     value = (int)(value * boost);
            // }

            value = constrain(binByPow[i].fftMag, MIN_FFT_DB, MAX_FFT_DB);
            value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, (BAR_HEIGHT * 10) + 9); // fins a 89

            _ThePanel.DrawBar(bin - 1, value, binByPow[i].brightness);
        }*/
    // DrawParametric();

    // FastLED.show();
    //_ThePanel.IncBaseHue();
}

DrawParametricData* __dpd = nullptr;
void DrawParametric(MsgAudio2Draw& mad)
{
    // static ParametricCurve s_TheCurrentCurve;
    // static DemoModeParams s_TheDemoParams;
    // static bool s_initialized = false;
    // static const uint16_t s_numCoords = 252;
    // static uint16_t s_frameNum = 0;
    // static float s_steps[s_numCoords];
    // static float s_delta = 0.0f;
    // // static float s_deltaRatio = 0.01f;
    // // static float s_deltaRatioTotal = 1.0f;
    // static uint8_t s_hue = 0;
    // static float xMag = 1.0;
    // static float yMag = 1.0;
    // static bool reachingMag = false;
    // static bool rotating = false;
    // static uint32_t shiftCount = 0;
    // static int16_t textPos = THE_PANEL_WIDTH - 2;
    // static int16_t inc = -1;

    if (!__dpd) {
        __dpd = new DrawParametricData();
    }

    bool newPhase = false;
    // static bool alreadyDrawedText = false;
    // FastLED.clear();

    if (!__dpd->initialized) {
        float every = (float)MOVING_PARAMETRIC_STEPS / (float)MOVING_PARAMETRIC_POINTS;
        for (uint16_t i = 0; i < MOVING_PARAMETRIC_POINTS; i++) {
            __dpd->TheCurrentCurve.initialPoints[i] = (uint16_t)(every * i);
        }
        double menysPI = -PI;
        for (uint16_t i = 0; i < MOVING_PARAMETRIC_STEPS; i++) {
            __dpd->steps[i] = menysPI + (i * 0.025); // 0.025=2Pi/252. 252 son el num d'steps
            // s_TheCurrentCurve.xCoord[i] = 31.1f * sin(2 * s_steps[i] + HALF_PI) + 32.0f;
        }
        __dpd->initialized = true;
    }

    // Calculate max BASS power among first bars
    _1stBarValue = 0;
    for (uint16_t i = 4; i < THE_PANEL_WIDTH / 5; i++) {
        auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
        if (_1stBarValue < value) {
            _1stBarValue = value;
        }
    }

    if ((_DemoMode && _Connected2Wifi) || (_TheFrameNumber > 200)) {
        if (_DemoModeFrame == 0) {
            __dpd->TheDemoParams.currentPhase = 0;
            __dpd->TheDemoParams.numPhases = 3;
            __dpd->TheDemoParams.phaseMagsx[0] = 1.0;
            __dpd->TheDemoParams.phaseMagsx[1] = 1.0;
            __dpd->TheDemoParams.phaseMagsx[2] = 1.0;
            __dpd->TheDemoParams.phaseMagsx[3] = 2.0;
            __dpd->TheDemoParams.phaseMagsx[4] = 3.0;
            __dpd->TheDemoParams.phaseMagsy[0] = 1.0;
            __dpd->TheDemoParams.phaseMagsy[1] = 2.0;
            __dpd->TheDemoParams.phaseMagsy[2] = 3.0;
            __dpd->TheDemoParams.phaseMagsy[3] = 3.0;
            __dpd->TheDemoParams.phaseMagsy[4] = 4.0;

            __dpd->xMag = __dpd->TheDemoParams.phaseMagsx[0];
            __dpd->yMag = __dpd->TheDemoParams.phaseMagsy[0];
            __dpd->reachingMag = false;
            __dpd->rotating = true;
        }

        if (__dpd->reachingMag) {
            if (__dpd->xMag < __dpd->TheDemoParams.phaseMagsx[__dpd->TheDemoParams.currentPhase]) {
                __dpd->xMag += 0.01;
            }
            if (__dpd->yMag < __dpd->TheDemoParams.phaseMagsy[__dpd->TheDemoParams.currentPhase]) {
                __dpd->yMag += 0.01;
            }
            if (__dpd->xMag >= __dpd->TheDemoParams.phaseMagsx[__dpd->TheDemoParams.currentPhase] && __dpd->yMag >= __dpd->TheDemoParams.phaseMagsy[__dpd->TheDemoParams.currentPhase]) {
                __dpd->reachingMag = false;
                __dpd->rotating = true;
            }
        }
        if (__dpd->rotating) {
            if (_DemoModeFrame >= ((uint32_t)(__dpd->TheDemoParams.currentPhase + 1) * 250)) {
                __dpd->TheDemoParams.currentPhase++;
                newPhase = true;
                __dpd->alreadyDrawedText = false;
                if (__dpd->TheDemoParams.currentPhase < MAX_DEMO_PHASES) {
                    __dpd->rotating = false;
                    __dpd->reachingMag = true;
                } else {
                    // _DemoModeFrame = 0;
                    // s_TheDemoParams.currentPhase = 0;
                    // xMag = s_TheDemoParams.phaseMagsx[0];
                    // yMag = s_TheDemoParams.phaseMagsy[0];
                    _DemoMode = false;
                    _u8g2.setFont(u8g2_font_princess_tr);
                    log_d("DeMo MoDe FINISH");
                    if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
                        _UpdateCurrentNow = true;
                    }
                }
            }
            _DemoModeFrame++;
        }
    }

    if (__dpd->frameNum == 0) {
        /* x = 31.1*sin(3*step+pi/2)+32
           y = 15.1*sin(2*step)+16*/
        for (uint16_t i = 0; i < MOVING_PARAMETRIC_STEPS; i++) {
            __dpd->TheCurrentCurve.xCoord[i] = 31.5f * sin(__dpd->xMag * __dpd->steps[i] + HALF_PI) + 32.0f;
            __dpd->TheCurrentCurve.yCoord[i] = 15.5f * sin(__dpd->yMag * __dpd->steps[i] + __dpd->delta) + 16.0f;
        }
        __dpd->delta += 0.025;
        if (__dpd->delta >= TWO_PI) {
            __dpd->delta = 0.0f;
        }
        // shiftCount = 0;
        //  if (s_deltaRatioTotal > 6.0 || s_deltaRatioTotal < 1.0) {
        //      s_deltaRatio = (-s_deltaRatio);
        //  }
        //  s_deltaRatioTotal += s_deltaRatio;
    }
    uint8_t intensity = std::max((uint8_t)(100), _1stBarValue);

    for (uint16_t i = 0; i < MOVING_PARAMETRIC_POINTS; i++) {
        if (__dpd->rotating && (_DemoModeFrame % 2) == 0) {
            __dpd->TheCurrentCurve.initialPoints[i] = (__dpd->TheCurrentCurve.initialPoints[i] + 1) % MOVING_PARAMETRIC_STEPS;
            // shiftCount = (shiftCount + 1) % s_numCoords;
        }
        uint16_t coord = __dpd->TheCurrentCurve.initialPoints[i];
        // uint8_t intensity = max(DEFAULT_MILLIS, _1stBarValue);
        //  int value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        //   int value = constrain(_1stBarValue, MIN_FFT_DB, MAX_FFT_DB);
        //   value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
        //_TheLeds[_TheMapping.XY(round(s_TheCurrentCurve.xCoord[coord]), round(s_TheCurrentCurve.yCoord[coord]))] = CHSV(HSVHue::HUE_BLUE, 255, (uint8_t)value);

        // if (i > shiftCount && i < ((shiftCount + (s_numCoords / 2)) % s_numCoords)) {
        //     intensity = intensity / 2;
        // }
        // if (i < MOVING_PARAMETRIC_POINTS/2) {
        //     intensity = intensity / 2;
        // }

        _TheLeds[_TheMapping.XY(round(__dpd->TheCurrentCurve.xCoord[coord]), round(__dpd->TheCurrentCurve.yCoord[coord]))] = CHSV(HSVHue::HUE_YELLOW + _1stBarValue / 3, 255, intensity); //(uint8_t)value);
    }

    // s_frameNum = 0;
    // s_frameNum= (s_frameNum + 1) % 3;

    // i ara pintem el text
    //_u8g2.clearBuffer();
    std::string test;
    if (__dpd->TheDemoParams.currentPhase == 0 && !_Connected2Wifi && !__dpd->alreadyDrawedText) {
        _u8g2.setFont(u8g2_font_oskool_tr); // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr
        test = "Iniciant FlipaLeds...";
        _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
        __dpd->alreadyDrawedText = true;
    } else if (__dpd->TheDemoParams.currentPhase < (MAX_DEMO_PHASES - 2) && _Connected2Wifi && !__dpd->alreadyDrawedText) {
        _u8g2.clearBuffer();
        //_u8g2.setFont(u8g2_font_profont10_tr); // u8g2_font_tom_thumb_4x6_mn); //u8g2_font_unifont_t_emoticons big emoticons 14pix
        test = "Connectat a la wiFi!";
        _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
        __dpd->alreadyDrawedText = true;
    } else if (__dpd->TheDemoParams.currentPhase >= (MAX_DEMO_PHASES - 2)) {
        if (_Connected2Wifi && !__dpd->alreadyDrawedText) {
            _u8g2.clearBuffer();
            test = Utils::string_format("IP=[%s]", WiFi.localIP().toString().c_str());
            //_u8g2.setFont(u8g2_font_princess_tr); // u8g2_font_tom_thumb_4x6_mn);
            _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
            __dpd->alreadyDrawedText = true;
        } else if (!_Connected2Wifi && newPhase) {
            _u8g2.clearBuffer();
            test = Utils::string_format("Sense WiFi...");
            //_u8g2.setFont(u8g2_font_princess_tr); // u8g2_font_tom_thumb_4x6_mn);
            _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
        }
    }
    // botmaker_te --> 16x16 funny
    // font_princess_tr --> maca, de princesses
    // emoticons21 --> funny emoticons
    // u8g2_font_tom_thumb_4x6_tn --> es veu bé, però els ":" ocupen un full char
    // u8g2_font_eventhrees_tr --> guay però molt petita, 3x3
    // u8g2_font_micro_tn --> 3x5 molt guay pero ocupa 3 pixels cada char.
    // _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 2)-1, test.c_str());
    //   _u8g2.setFont(u8g2_font_micro_tn); // u8g2_font_tom_thumb_4x6_tn   u8g2_font_blipfest_07_tn);
    //   _u8g2.drawStr(6, 12, theTime.c_str());
    //_ThePanel.SetBaseHue(HSVHue::HUE_YELLOW);
    _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 2, 3, __dpd->textPos, 0, __dpd->hue, intensity);

    __dpd->textPos += __dpd->inc;
    if (__dpd->textPos < (-THE_PANEL_WIDTH * 2)) {
        __dpd->inc = 1;
    } else if (__dpd->textPos > THE_PANEL_WIDTH) {
        __dpd->inc = (-1);
    }
    __dpd->hue++;

    if (!_DemoMode && __dpd) {
        delete __dpd;
        __dpd = nullptr;
    }
}

void DrawWave(MsgAudio2Draw& mad)
{
    struct WavePoint {
        uint16_t ledIndex;
        CHSV thePixel;
    };
    static WavePoint _WaveBuffer[MAX_FADING_WAVES][THE_PANEL_WIDTH]; // circular buffer of waves
    static uint8_t _LasttWaveIndex = 0; // index of the circular buffer, apunta a la wave que toca pintar
    static int16_t _OffsetMv = -5; // adjust wave scale so it does not appears to flat or clipping
    static auto _lastIncrease = millis();
    static uint16_t _maxValueVeryHi = 0; // max value drawn since _lastIncrease
    static uint16_t _maxValueHi = 0; // max value drawn since _lastIncrease

    uint8_t height = _TheMapping.GetHeight() - 1;
    uint16_t width = _TheMapping.GetWidth();
    uint16_t i;
    int16_t value;

    if (WITH_MEMS_MIC) { // Escalem la ona a "mic analògic 9814"
        int16_t valueOrig;
        int16_t scale = 10 + _OffsetMv; // max value of _OffsetMv=(-9)
        for (i = 0; i < mad.audioLenInSamples; i++) {
            valueOrig = mad.pAudio[i];
            // if (valueOrig < INT16_MIN / scale) {
            //     valueOrig = INT16_MIN / scale;
            // } else if (valueOrig > INT16_MAX / scale) {
            //     valueOrig = INT16_MAX / scale;
            // }
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
    for (i = 0; i < (width / 2); i++) {
        if (mad.pAudio[i] > maxAmp) {
            maxAmp = mad.pAudio[i];
            iMaxAmp = i;
        }
    } // ja tenim l'index del pic de la muntanya mes gran. Ara busquem a on creuem per 0
    for (i = iMaxAmp; i < width; i++) {
        if (mad.pAudio[i] <= INPUT_0_VALUE) {
            pas0 = i;
            break;
        }
    }

    CHSV myValue;
    myValue.setHSV(HSVHue::HUE_PURPLE, 255, 70);

    uint8_t numFadingWaves = 1;
    if (_TheDrawStyle != DRAW_STYLE::BARS_WITH_TOP) {
        // numFadingWaves = 2;
        myValue.setHSV(HSVHue::HUE_AQUA, 128, 70);
    }
    int16_t numValuesHi = 0;
    int16_t numValuesVeryHi = 0;
    for (i = 0; i < width; i++) {
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
        if (value >= height - 4) {
            numValuesVeryHi++;
        } else if (value >= height - 8) {
            numValuesHi++;
        }

        // value = min((int16_t)max(value, (int16_t)0), (int16_t)height);
        _WaveBuffer[_LasttWaveIndex][i].ledIndex = _TheMapping.XY(i, value);
        _WaveBuffer[_LasttWaveIndex][i].thePixel = myValue;
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
        if (_maxValueVeryHi > (THE_PANEL_WIDTH / 6) && _OffsetMv > (-9)) {
            _OffsetMv = _OffsetMv - ((_OffsetMv > (-5)) ? 2 : 1);
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("DEC WaveScale=%d, ValuesHi=%d ValuesVeryHi=%d", 10 + _OffsetMv, _maxValueHi, _maxValueVeryHi).c_str());
        } else if (_maxValueHi < (THE_PANEL_WIDTH / 5) && _OffsetMv < 7) {
            _OffsetMv++;
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("INC WaveScale=%d, ValuesHi=%d ValuesVeryHi=%d", 10 + _OffsetMv, _maxValueHi, _maxValueVeryHi).c_str());
        }
        // else {
        //     _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("STAY --> Offset=%d ValuesHi=%d ValuesVeryHi=%d", _OffsetMv, _maxValueHi, _maxValueVeryHi).c_str());
        // }

        _maxValueVeryHi = 0;
        _maxValueHi = 0;
        _lastIncrease = millis();
    }

    if (numFadingWaves > 1) { // fem desapareixer la 1era wave, que serà la última ara
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

    if (numFadingWaves > 1) { // fem desapareixer la 1era wave, que serà la última ara
        _LasttWaveIndex = (_LasttWaveIndex + 1) % numFadingWaves;
    }

    //_ThePanel.IncBaseHue();
}

void DrawMatrixFFT(MsgAudio2Draw& mad)
{
    const uint8_t numLines = mad.sizeFftMagVector / THE_PANEL_WIDTH;
    uint8_t currentLine = THE_PANEL_HEIGHT - 1;
    uint16_t currentBin = 0;
    uint8_t x;

    for (uint8_t iLine = 0; iLine < numLines; iLine++) {
        for (uint16_t xPix = 0; xPix < THE_PANEL_WIDTH; xPix++, currentBin++) {
            auto value = constrain(mad.pFftMag[currentBin], MIN_FFT_DB, MAX_FFT_DB);
            value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, 255);
            // if (value < 25) {
            //     value = 0;
            // } else {
            //     value -= 25;
            // }
            if (value < 3) { // black? el fem transparent :)
                continue;
            }

            if (iLine % 2 == 0) {
                x = xPix;
            } else {
                x = THE_PANEL_WIDTH - xPix - 1;
            }
            _TheLeds[_TheMapping.XY(x, currentLine)] = CHSV(HSVHue::HUE_PURPLE + (value / 2), 255, (uint8_t)value);
        }
        currentLine--;
    }
}

void DrawDiscoLights(MsgAudio2Draw& mad)
{
    // 32-64-128-256-512  --> 192Hz 384Hz 768Hz 1536Hz 3072Hz
    const uint16_t bins[] = { 110 / HZ_PER_BIN, 240 / HZ_PER_BIN, 500 / HZ_PER_BIN, 1500 / HZ_PER_BIN, mad.sizeFftMagVector };
    const uint8_t max_circle = 6;
    const uint8_t totalLights = sizeof(bins) / sizeof(uint16_t);
    const HSVHue hues[totalLights] = { HSVHue::HUE_RED, HSVHue::HUE_BLUE, HSVHue::HUE_GREEN, HSVHue::HUE_PINK, HSVHue::HUE_YELLOW };
    const uint8_t intensityOff[totalLights] = { 40, 45, 40, 50, 50 };
    uint8_t dbRange = abs((MAX_FFT_DB - MIN_FFT_DB));
    const uint8_t dbs_x_circle = abs((MAX_FFT_DB - MIN_FFT_DB) / (int)(max_circle));
    uint16_t currentBin = 0;
    // uint8_t numLight = 0;

    // Calculate max BASS power among first bars
    _1stBarValue = 0;
    for (uint16_t i = 0; i < THE_PANEL_WIDTH / 5; i++) {
        auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
        if (_1stBarValue < value) {
            _1stBarValue = value;
        }
    }

    for (uint8_t numLight = 0; numLight < totalLights; numLight++) {
        int8_t maxDb = -100;
        while (currentBin < bins[numLight]) {
            if (mad.pFftMag[currentBin] > maxDb) {
                maxDb = mad.pFftMag[currentBin];
            }
            currentBin++;
        }
        for (int8_t currDbs = MIN_FFT_DB, nCircle = 0; nCircle < max_circle; currDbs += dbs_x_circle, nCircle++) {
            if (currDbs > (MIN_FFT_DB + 5) && currDbs <= maxDb) {
                _ThePanel.DrawCircle((numLight * 12) + max_circle + 1, 17, nCircle, CHSV(hues[numLight], 255, (nCircle * 10) + 70)); // 255/6 circles= 42
            } else {
                _ThePanel.DrawCircle((numLight * 12) + max_circle + 1, 17, nCircle, CHSV(hues[numLight], 160, intensityOff[numLight])); // 255/6 circles= 42
            }
        }
    }
}

void DrawCurrentGraph(MsgAudio2Draw& mad)
{
    uint8_t maxV = GetMapMaxPixels();
    uint8_t everyKwh = GetPixelsPerKwh(maxV);
    static bool published = false;
    static uint8_t _incBoleta = 0;

    if (!published) {
        _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("maxV=%d, every=%d", maxV, everyKwh).c_str());
        published = true;
    }

    // Calculate max BASS power among first bars
    _1stBarValue = 0;
    for (uint16_t i = 2; i < THE_PANEL_WIDTH / 5; i++) {
        auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 5, 200);
        if (_1stBarValue < value) {
            _1stBarValue = value;
        }
    }

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
            break; // la línia de més amunt no la pintem
        }

        for (uint8_t i = 0 + _incBoleta; i < THE_PANEL_WIDTH; i += 4) {
            _TheLeds[_TheMapping.XY(i, THE_PANEL_HEIGHT - 1 - j)] += kw;
        }
    }
    // i ara pintem les barres
    if (_pLectures) {
        long mapValue = 0;
        uint8_t maxValue = 0;
        uint8_t intensity = max((int)100, (int)_1stBarValue);
        int intensityBar;
        for (uint8_t i = 0; i < THE_PANEL_WIDTH; i++) {
            mapValue = map(_pLectures[i].valorEnLeds, 0, _MapMaxWhToPixels, 0, maxV);

            // pintem "barres"
            intensityBar = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
            intensityBar = map(intensityBar, MIN_FFT_DB, MAX_FFT_DB + 5, 0, 140);
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
        }
        // pintem el led que es va movent per sobre la linia de valors d'electricitat
        // uint8_t fiufiu = (uint8_t)(_TheFrameNumber % (THE_PANEL_WIDTH - 1));
        // mapValue = map(_pLectures[fiufiu].valorEnLeds, 0, _MapMaxWhToPixels, 0, maxV);
        // _TheLeds[_TheMapping.XY(fiufiu + 1, THE_PANEL_HEIGHT - 2 - mapValue)] += CHSV(HSVHue::HUE_YELLOW, 255, 200);

        if (maxValue < 14 && _MapMaxWhToPixels > 1500) {
            _MapMaxWhToPixels -= 500;
        } else if (maxValue >= 18 && _MapMaxWhToPixels < 6000) {
            _MapMaxWhToPixels += 500;
        }
    }
}

void DrawClock()
{
    static int baseHue = 0;

    // if (_NightMode && (_TheFrameNumber % 2) == 0) {
    //     return;
    // }

    struct tm timeinfo;
    if (_Connected2Wifi) {
        getLocalTime(&timeinfo);
    } else {
        time_t now = time(0); // secs since boot
        timeinfo = *localtime(&now);
    }
    std::string theTime;
    theTime = Utils::string_format("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    // static int count = 0;
    // if ((count % 25) == 0) {
    //     log_d("Time=%s", theTime.c_str());
    // }
    // count++;

    // botmaker_te --> 16x16 funny
    // font_princess_tr --> maca, de princesses
    // emoticons21 --> funny emoticons
    // u8g2_font_tom_thumb_4x6_tn --> es veu bé, però els ":" ocupen un full char
    // u8g2_font_eventhrees_tr --> guay però molt petita, 3x3
    // u8g2_font_micro_tn --> 3x5 molt guay pero ocupa 3 pixels cada char.
    _u8g2.clearBuffer();
    //    _u8g2.setFont(u8g2_font_squeezed_r6_tn); // u8g2_font_tom_thumb_4x6_mn);
    _u8g2.drawStr(THE_PANEL_WIDTH - CLOCK_HORIZ_PIXELS, CLOCK_VERT_PIXELS, theTime.c_str());
    //   _u8g2.setFont(u8g2_font_micro_tn); // u8g2_font_tom_thumb_4x6_tn   u8g2_font_blipfest_07_tn);
    //   _u8g2.drawStr(6, 12, theTime.c_str());
    uint8_t intensity;
    if (_NightMode) {
        _ThePanel.SetBaseHue(HSVHue::HUE_BLUE);
        intensity = 164;
    } else {
        _ThePanel.SetBaseHue((uint8_t)(_TheFrameNumber / 4));
        intensity = max((int)164, (int)_1stBarValue);
    }
    _ThePanel.DrawScreenBuffer(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), THE_PANEL_WIDTH, 2, baseHue++, intensity);
    //        FastLED.show();
}

//*****************************************************
// Test drawing methods
//*****************************************************

// uint8_t thisSpeed = 3;
// uint8_t initial = 1;
int _delayFrame = 100;

// void DrawParametricHardcoded()
// {
//     static uint16_t currPos = 0;
//     static uint16_t sizePoses = 8; // sizeof(__2to3PiHalf.initialPoints) / sizeof(uint16_t);
//     static HSVHue theHues[8] = { HSVHue::HUE_AQUA, HSVHue::HUE_BLUE, HSVHue::HUE_GREEN, HSVHue::HUE_ORANGE, HSVHue::HUE_PINK, HSVHue::HUE_PURPLE, HSVHue::HUE_RED, HSVHue::HUE_YELLOW };

//     //    FastLED.clear();
//     //_TheLeds.fadeLightBy(90);

//     // for (int i = 0; i < sizePoses; i++) {
//     //     _TheLeds[_TheMapping.XY(
//     //         round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
//     //         round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
//     //         = CHSV(theHues[i], 255, 200);
//     // }
//     for (int i = 0; i < sizePoses; i++) {
//         // float x = __2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
//         // float y = __2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
//         // int x1 = (int)x + 1;
//         // int y1 = (int)y + 1;
//         // float x1Percent = x - (int)x;
//         // float x0Percent = 1.0 - x1Percent;
//         // float y1Percent = y - (int)y;
//         // float y0Percent = 1.0 - x1Percent;
//         // float c0Percent = (x0Percent + y0Percent) / 2;
//         // float c1Percent = (x1Percent + y1Percent) / 2;

//         // if (x1 >= _TheMapping.GetWidth() || y1 >= _TheMapping.GetHeight()) {
//         //     _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255);
//         // } else {
//         //     _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255 * c0Percent);
//         //     _TheLeds[_TheMapping.XY((int)x1, (int)y1)] = CHSV(theHues[i], 255, 255 * c1Percent);
//         // }

//         _TheLeds[_TheMapping.XY(
//             round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
//             round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
//             = CHSV(theHues[i], 255, 200);
//     }

//     ++currPos;
//     // currPos = currPos % 252;
//     //   FastLED.show();
// }

/*
void DrawImage()
{
    static int pos = 0;
    static int temp = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 16; j++) {
            _TheLeds[_TheMapping.XY((i + pos) % 33, j)] = __imgArletErola2[i][j];
            napplyGamma_video(_TheLeds[_TheMapping.XY((i + pos) % 33, j)], 1.8f);
        }
    }
    pos++;
    // FastLED.show();
}

#include <math.h>
constexpr int pixelCount = 800;
uint8_t sparkHue = HSVHue::HUE_AQUA; // .5; //.2 => fire .1 => redish fire .5 => aqua
int sparkSaturation = 1;
constexpr int numSparks = 1 + (pixelCount / 20);
float decay = 0.995;
uint8_t maxSpeed = 100;
float newThreshhold = 0.03;

float sparks[numSparks];
float sparkX[numSparks];
float pixels[pixelCount];

uint16_t frames = 0;

void DrawSparks()
{
    if (++frames > 500) {
        sparkHue = random8();
        frames = 0;
    }

    float delta = 3; // po ejemplo

    for (int i = 0; i < pixelCount; i++) {
        pixels[i] = pixels[i] * 0.92;
    }

    for (int i = 0; i < numSparks; i++) {
        if (sparks[i] >= -newThreshhold && sparks[i] <= newThreshhold) {
            sparks[i] = ((int)(maxSpeed / 2) - (int)random8(maxSpeed)) / 100.0; // random entre -maxspeed/2 i maxspeed/2, entre 100.0
            sparkX[i] = random16(pixelCount);
        }

        sparks[i] *= decay;
        sparkX[i] += sparks[i] * delta;

        if (sparkX[i] >= pixelCount) {
            sparkX[i] = 0;
        }

        if (sparkX[i] < 0) {
            sparkX[i] = pixelCount - 1;
        }

        pixels[(int)floor(sparkX[i])] += sparks[i]; // com mes "calent" més ràpid
    }

    // log_v("-------------");
    // uint8_t maxv = 0;
    // float maxf = 0.0;
    for (int i = 0; i < pixelCount; i++) {
        float f = (pixels[i] * pixels[i] * 5.0) * 255.0;
        uint8_t v = f > 255.0 ? 255 : (uint8_t)f;
        _TheLeds[i] = CHSV(sparkHue, 255, v);
        // log_v("Pixel[%d]=%d", i, v);
        // if (v > maxv) {
        //     maxv = v;
        // }
        // if (pixels[i] > maxf) {
        //     maxf = pixels[i];
        // }
    }
    // log_v("maxV=%d maxF=%2.3f", maxv, maxf);

    FastLED.show();
    // delay(5);
}
*/
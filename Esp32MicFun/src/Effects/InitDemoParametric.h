
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
    // _1stBarValue = 0;
    // for (uint16_t i = 4; i < THE_PANEL_WIDTH / 5; i++) {
    //     auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
    //     value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
    //     if (_1stBarValue < value) {
    //         _1stBarValue = value;
    //     }
    // }

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

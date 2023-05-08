
#define CLOCK_HORIZ_PIXELS 41
#define CLOCK_VERT_PIXELS 7

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

    _ThePanel.SetBaseHue((HSVHue::HUE_ORANGE - (MAX_INC / 3)) + actualInc);
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
        int8_t decPower = 0;
        if (i < 18) {
            decPower = 14;
        }
        value = constrain(mad.pFftMag[i], (int)MIN_FFT_DB + (int)(_pianoMode ? 0 : 0) + decPower, MAX_FFT_DB);
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        // value = constrain(mad.pFftMag[i], MIN_FFT_DB-5, MAX_FFT_DB);
        value = map(value, (int)MIN_FFT_DB + (int)(_pianoMode ? 0 : 0) + decPower, MAX_FFT_DB, 0, 255);
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

    assert(BAR_HEIGHT > 1);
    uint8_t minBoostBin = (uint8_t)(numItems * 0.33); // the first 13 bars in 33 width panel
    constexpr float maxTrebleBoost = 30.0;
    constexpr float minBassBoost = 1.0;
    float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)numItems);

    FastLED.clear();
    // DrawImage();
    //_ThePanel.SetBaseHue();
    _ThePanel.SetBaseHue(HSVHue::HUE_BLUE);
    for (uint16_t i = 0; i < numItems; i++) {
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, (BAR_HEIGHT * 10) + 9); // fins a 89

        _ThePanel.DrawBar(i, value, 200);
        if (i <= 5 && _1stBarValue < value) {
            _1stBarValue = value;
        }
    }
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

void DrawParametric(MsgAudio2Draw& mad)
{
    static ParametricCurve s_TheCurrentCurve;
    static DemoModeParams s_TheDemoParams;
    static bool s_initialized = false;
    static const uint16_t s_numCoords = 252;
    static uint16_t s_frameNum = 0;
    static float s_steps[s_numCoords];
    static float s_delta = 0.0f;
    // static float s_deltaRatio = 0.01f;
    // static float s_deltaRatioTotal = 1.0f;
    static uint8_t s_hue = 0;
    static float xMag = 1.0;
    static float yMag = 1.0;
    static bool reachingMag = false;
    static bool rotating = false;
    // FastLED.clear();

    if (!s_initialized) {
        float every = (float)s_numCoords / THE_PANEL_WIDTH;
        for (uint16_t i = 0; i < THE_PANEL_WIDTH; i++) {
            s_TheCurrentCurve.initialPoints[i] = (uint16_t)(every * i);
        }
        double menysPI = -PI;
        for (uint16_t i = 0; i < s_numCoords; i++) {
            s_steps[i] = menysPI + (i * 0.025); // 0.025=2Pi/252. 252 son el num d'steps
            // s_TheCurrentCurve.xCoord[i] = 31.1f * sin(2 * s_steps[i] + HALF_PI) + 32.0f;
        }
        s_initialized = true;
    }

    // Calculate max BASS power among first bars
    _1stBarValue = 0;
    for (uint16_t i = 0; i < THE_PANEL_WIDTH / 6; i++) {
        auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 10, 200);
        if (_1stBarValue < value) {
            _1stBarValue = value;
        }
    }

    if (_DemoMode && _Connected2Wifi) {
        if (_DemoModeFrame == 0) {
            s_TheDemoParams.currentPhase = 0;
            s_TheDemoParams.numPhases = 3;
            s_TheDemoParams.phaseMagsx[0] = 1.0;
            s_TheDemoParams.phaseMagsx[1] = 1.0;
            s_TheDemoParams.phaseMagsx[2] = 1.0;
            s_TheDemoParams.phaseMagsx[3] = 2.0;
            s_TheDemoParams.phaseMagsx[4] = 3.0;
            s_TheDemoParams.phaseMagsy[0] = 1.0;
            s_TheDemoParams.phaseMagsy[1] = 2.0;
            s_TheDemoParams.phaseMagsy[2] = 3.0;
            s_TheDemoParams.phaseMagsy[3] = 3.0;
            s_TheDemoParams.phaseMagsy[4] = 4.0;

            xMag = s_TheDemoParams.phaseMagsx[0];
            yMag = s_TheDemoParams.phaseMagsy[0];
            reachingMag = false;
            rotating = true;
        }

        if (reachingMag) {
            // _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Reaching x=[%2.2f] y=[%2.2f] frame=%d Phase=%d", xMag, yMag, _DemoModeFrame, s_TheDemoParams.currentPhase).c_str(), true);

            if (xMag < s_TheDemoParams.phaseMagsx[s_TheDemoParams.currentPhase]) {
                xMag += 0.01;
            }
            if (yMag < s_TheDemoParams.phaseMagsy[s_TheDemoParams.currentPhase]) {
                yMag += 0.01;
            }
            if (xMag >= s_TheDemoParams.phaseMagsx[s_TheDemoParams.currentPhase] && yMag >= s_TheDemoParams.phaseMagsy[s_TheDemoParams.currentPhase]) {
                reachingMag = false;
                rotating = true;
            }
        }
        if (rotating) {
            if (_DemoModeFrame >= ((uint32_t)(s_TheDemoParams.currentPhase + 1) * 300)) {
                s_TheDemoParams.currentPhase++;
                if (s_TheDemoParams.currentPhase < MAX_DEMO_PHASES) {
                    rotating = false;
                    reachingMag = true;
                } else {
                    _DemoMode = false;
                }
            }
            _DemoModeFrame++;
        }
    }

    if (s_frameNum == 0) {
        /* x = 31.1*sin(3*step+pi/2)+32
           y = 15.1*sin(2*step)+16*/
        for (uint16_t i = 0; i < s_numCoords; i++) {
            s_TheCurrentCurve.xCoord[i] = 31.1f * sin(xMag * s_steps[i] + HALF_PI) + 32.0f;
            s_TheCurrentCurve.yCoord[i] = 15.1f * sin(yMag * s_steps[i] + s_delta) + 16.0f;
        }
        s_delta += 0.025;
        if (s_delta >= TWO_PI) {
            s_delta = 0.0f;
        }
        // if (s_deltaRatioTotal > 6.0 || s_deltaRatioTotal < 1.0) {
        //     s_deltaRatio = (-s_deltaRatio);
        // }
        // s_deltaRatioTotal += s_deltaRatio;
    }
    for (uint16_t i = 0; i < THE_PANEL_WIDTH; i++) {
        if (rotating) {
            s_TheCurrentCurve.initialPoints[i] = (s_TheCurrentCurve.initialPoints[i] + 1) % s_numCoords;
        }
        uint16_t coord = s_TheCurrentCurve.initialPoints[i];
        // int value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        //  int value = constrain(_1stBarValue, MIN_FFT_DB, MAX_FFT_DB);
        //  value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
        //_TheLeds[_TheMapping.XY(round(s_TheCurrentCurve.xCoord[coord]), round(s_TheCurrentCurve.yCoord[coord]))] = CHSV(HSVHue::HUE_BLUE, 255, (uint8_t)value);
        _TheLeds[_TheMapping.XY(round(s_TheCurrentCurve.xCoord[coord]), round(s_TheCurrentCurve.yCoord[coord]))] = CHSV(HSVHue::HUE_YELLOW + _1stBarValue / 3, 255, _1stBarValue); //(uint8_t)value);
    }

    // s_frameNum = 0;
    // s_frameNum= (s_frameNum + 1) % 3;
    s_hue++;
}

void DrawWave(MsgAudio2Draw& mad)
{
    // static int8_t incValue = +1;
    // static int8_t actualInc = 0;
    // static const int8_t MAX_INC = 32;

    // if (_numFrames % 4 == 0) {
    //     actualInc += incValue;
    //     if (actualInc >= MAX_INC) {
    //         incValue = -1;
    //     } else if (actualInc == 0) {
    //         incValue = +1;
    //     }
    // }

    uint8_t height = _TheMapping.GetHeight() - 1;
    uint16_t width = _TheMapping.GetWidth();
    uint16_t i;
    int16_t value;

    // busquem el pass per "0" després de la muntanya més gran
    uint16_t pas0 = 0;
    uint16_t maxAmp = INPUT_0_VALUE;
    uint16_t iMaxAmp = 0;
    for (i = 0; i < (width / 2); i++) {
        if (mad.pAudio[i] > (maxAmp + 100)) {
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

    for (i = 0; i < width; i++) {
        value = constrain(mad.pAudio[pas0 + (i * 2)], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
        value += constrain(mad.pAudio[pas0 + (i * 2) + 1], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
        value = map(value / 2, INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, 0, height);

        // value = constrain(mad.pAudio[pas0 + (i * 3)], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
        // value += constrain(mad.pAudio[pas0 + (i * 3) + 1], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
        // value += constrain(mad.pAudio[pas0 + (i * 3) + 2], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
        // value = map(value / 3, INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, 0, height);

        // value = constrain(mad.pAudio[pas0 + i], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
        // value = map(value, INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, 0, height);
        _TheLeds[_TheMapping.XY(i, value)].setHSV(HSVHue::HUE_BLUE, 148, 48);
    }
    //_ThePanel.IncBaseHue();
}

void DrawClock()
{
    static int baseHue = 0;

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
    _ThePanel.SetBaseHue((uint8_t)(_TheFrameNumber / 4));
    _ThePanel.DrawScreenBuffer(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), THE_PANEL_WIDTH, 1, baseHue++, max((int)164, (int)_1stBarValue));
    //        FastLED.show();
}

//*****************************************************
// Test drawing methods
//*****************************************************

// uint8_t thisSpeed = 3;
// uint8_t initial = 1;
int _delayFrame
    = 100;

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

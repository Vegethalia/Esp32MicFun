
void DrawHorizSpectrogram(MsgAudio2Draw& mad)
{
    int16_t value = 0;

    //_TheLeds.fill_solid(CRGB(1,1,1));
    //   FastLED.clear();
    assert(BAR_HEIGHT > 1);
    const auto numItems = _TheMapping.GetHeight();
    uint8_t values[numItems];

    for (uint16_t i = 1; i <= _TheMapping.GetHeight(); i++) {
        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, 255);
        values[i - 1] = (uint8_t)value;
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
    assert(THE_PANEL_WIDTH > 1);
    const auto numItems = _TheMapping.GetWidth();
    uint8_t values[numItems];

    // uint8_t minBoostBin = (uint8_t)(numItems * 0.33); // the first 13 bars in 33 width panel
    // constexpr float maxTrebleBoost = 5.0;
    // constexpr float minBassBoost = 1.0;
    // float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)numItems);
    _1stBarValue = 0;
    for (uint16_t i = 1; i <= _TheMapping.GetWidth(); i++) {
        value = constrain(mad.pFftMag[i], MIN_FFT_DB - 5, MAX_FFT_DB);
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        // value = constrain(mad.pFftMag[i], MIN_FFT_DB-5, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB - 5, MAX_FFT_DB, 0, 255);
        values[i - 1] = (uint8_t)value;
        if (i <= 3 && _1stBarValue < value) {
            _1stBarValue = value;
        }
    }
    if (_WithClock) {
        _ThePanel.PushLine(values, THE_PANEL_WIDTH - 27, 5);
        //_ThePanel.PushLine(values, 6, 12);
    } else {
        _ThePanel.PushLine(values);
    }

    // FastLED.show();
    // lastFrame = millis();
}

void DrawLedBars(MsgAudio2Draw& mad)
{
    // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);

    // static uint32_t lastSuperBass = 0;
    // static uint32_t lastLowBass = 0;
    // static uint32_t lastTouch = 0;
    // static constexpr int lowBass = ((MIN_FFT_DB + MAX_FFT_DB) / 2);
    // static constexpr int hiBass = (MIN_FFT_DB / 6);
    // static constexpr int touchMinMs = 200;
    // static constexpr int touchMaxMs = 500;
    // static constexpr int minTimeBetweenTouches = 1000;
    // static uint8_t state = 0; // 0=waiting for low. 1=with hi bass. 2=with low after hi

    uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
    uint8_t maxBassValue = 0;
    int16_t value = 0;

    if (VISUALIZATION == FftPower::AUTO34) {
        maxIndex = 34;
    }
    //_TheLeds.fill_solid(CRGB(1,1,1));
    FastLED.clear();
    assert(BAR_HEIGHT > 1);
    uint8_t minBoostBin = (uint8_t)(maxIndex * 0.33); // the first 13 bars in 33 width panel
    constexpr float maxTrebleBoost = 30.0;
    constexpr float minBassBoost = 1.0;
    float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)maxIndex);

    /*    bool isHiBass = false, isLowBass = false;
        auto now = millis();
        if (mad.pFftMag[1] >= hiBass) { // || mad.pFftMag[2] >= hiBass) {
            isHiBass = true;
            lastSuperBass = now;
        } else if (mad.pFftMag[1] < lowBass) { //|| mad.pFftMag[2] < lowBass) {
            isLowBass = true;
            lastLowBass = now;
        }
        if (state == 0 && isHiBass && (now - lastLowBass) > touchMinMs && (now - lastLowBass) < touchMaxMs) {
            state = 1;
        } else if (state == 1 && isLowBass && (now - lastTouch) > minTimeBetweenTouches) {
            //_ThePanel.IncBaseHue(41);
            lastTouch = now;
            state = 0;
        } else {
            state = 0;
        }
    */
    // // lets find out the max power bins
    // struct orderedBinByPow {
    //     uint16_t binNum;
    //     int32_t fftMag;
    //     uint8_t brightness;
    // };
    // std::vector<orderedBinByPow> binByPow;
    // for (uint16_t i = 0; i < maxIndex; i++) {
    //     binByPow.push_back({ i, mad.pFftMag[i], 32 });
    // }
    // binByPow[0].fftMag = MIN_FFT_DB;
    // std::sort(binByPow.begin(), binByPow.end(),
    //     [](orderedBinByPow& a, orderedBinByPow& b) {
    //         return a.fftMag < b.fftMag;
    //     });
    // for (uint16_t i = 0; i < maxIndex; i++) {
    //     if (i < 4) {
    //         binByPow[i].brightness = 200;
    //     } else if (i < 8) {
    //         binByPow[i].brightness = 160;
    //     } else if (i < 12) {
    //         binByPow[i].brightness = 120;
    //     } else if (i < 16) {
    //         binByPow[i].brightness = 100;
    //     } else if (i < 20) {
    //         binByPow[i].brightness = 80;
    //     } else {
    //         binByPow[i].brightness = 64;
    //     }
    // }

    // DrawImage();
    //_ThePanel.SetBaseHue();
    for (uint16_t i = 1; i < maxIndex; i++) {
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, (BAR_HEIGHT * 10) + 9); // fins a 89

        _ThePanel.DrawBar(i - 1, value, 200);
        if (i == 1) {
            _1stBarValue = map(value, 0, (BAR_HEIGHT * 10) + 9, 0, 255); // fins a 89;
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

void DrawClock()
{
    static int baseHue = 0;
    String theTime = _TheNTPClient.getFormattedTime();
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
    _u8g2.drawStr(THE_PANEL_WIDTH-27, 6, theTime.c_str());
    //   _u8g2.setFont(u8g2_font_micro_tn); // u8g2_font_tom_thumb_4x6_tn   u8g2_font_blipfest_07_tn);
    //   _u8g2.drawStr(6, 12, theTime.c_str());
    _ThePanel.DrawScreenBuffer(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), THE_PANEL_WIDTH, 1, baseHue++, max((int)164, (int)_1stBarValue));
    //        FastLED.show();
}

//*****************************************************
// Test drawing methods
//*****************************************************

// uint8_t thisSpeed = 3;
// uint8_t initial = 1;
int _delayFrame = 100;

void DrawParametric()
{
    static uint16_t currPos = 0;
    static uint16_t sizePoses = 8; // sizeof(__2to3PiHalf.initialPoints) / sizeof(uint16_t);
    static HSVHue theHues[8] = { HSVHue::HUE_AQUA, HSVHue::HUE_BLUE, HSVHue::HUE_GREEN, HSVHue::HUE_ORANGE, HSVHue::HUE_PINK, HSVHue::HUE_PURPLE, HSVHue::HUE_RED, HSVHue::HUE_YELLOW };

    //    FastLED.clear();
    //_TheLeds.fadeLightBy(90);

    // for (int i = 0; i < sizePoses; i++) {
    //     _TheLeds[_TheMapping.XY(
    //         round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
    //         round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
    //         = CHSV(theHues[i], 255, 200);
    // }
    for (int i = 0; i < sizePoses; i++) {
        // float x = __2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
        // float y = __2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
        // int x1 = (int)x + 1;
        // int y1 = (int)y + 1;
        // float x1Percent = x - (int)x;
        // float x0Percent = 1.0 - x1Percent;
        // float y1Percent = y - (int)y;
        // float y0Percent = 1.0 - x1Percent;
        // float c0Percent = (x0Percent + y0Percent) / 2;
        // float c1Percent = (x1Percent + y1Percent) / 2;

        // if (x1 >= _TheMapping.GetWidth() || y1 >= _TheMapping.GetHeight()) {
        //     _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255);
        // } else {
        //     _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255 * c0Percent);
        //     _TheLeds[_TheMapping.XY((int)x1, (int)y1)] = CHSV(theHues[i], 255, 255 * c1Percent);
        // }

        _TheLeds[_TheMapping.XY(
            round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
            round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
            = CHSV(theHues[i], 255, 200);
    }

    ++currPos;
    // currPos = currPos % 252;
    //   FastLED.show();
}

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

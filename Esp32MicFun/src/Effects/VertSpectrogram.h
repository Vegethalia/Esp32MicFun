
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

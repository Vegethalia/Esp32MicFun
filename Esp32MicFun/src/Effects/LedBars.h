
void DrawLedBars(MsgAudio2Draw& mad)
{
    const auto numItems = _TheMapping.GetWidth();
    uint8_t maxBassValue = 0;
    int16_t value = 0;
    uint8_t maxHeight = min(16, BAR_HEIGHT); // 18
    uint8_t maxHeightScaled = (maxHeight * 10) + 9;
    static uint8_t __binGrouping = 4; // group ins every this number
    static ALTERDRAW __alterDraw = ALTERDRAW::ODD;

    if (_TheLastKey != GEN_KEY_PRESS::KEY_NONE) {
        if (_TheLastKey == GEN_KEY_PRESS::KEY_INC1) {
            __alterDraw = (((uint8_t)__alterDraw < 2) ? (ALTERDRAW)(__alterDraw + 1) : ALTERDRAW::NO_ALTER);
        } else if (_TheLastKey == GEN_KEY_PRESS::KEY_DEC1) {
            __alterDraw = (((uint8_t)__alterDraw > 0) ? (ALTERDRAW)(__alterDraw - 1) : ALTERDRAW::ALTERNATE);
        } else if (_TheLastKey == GEN_KEY_PRESS::KEY_DEC2 && __binGrouping > 1) {
            __binGrouping--;
        } else if (_TheLastKey == GEN_KEY_PRESS::KEY_INC2 && __binGrouping < 8) {
            __binGrouping++;
        }
        std::string msg = Utils::string_format("BinGrouping=%d", (int)__binGrouping);
        _ThePubSub.publish(TOPIC_DEBUG, msg.c_str(), false);

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
        _ThePanel.SetBaseHue((HSVHue)_TheDesiredHue);
    }

    for (uint16_t i = 0; i < numItems; i++) {
        // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
        //     auto boost = 1.0f + (i * freqBoost);
        //     value = (int)(value * boost);
        // }

        // value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        //value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, maxHeightScaled); // fins a 89 // 169
        value = map(mad.pDBs[i], 0, 255, 0, maxHeightScaled); // fins a 89 // 169

        if (__binGrouping > 1) {
            int32_t avg = value;
            int32_t max = value;
            for (uint8_t j = 1; j < __binGrouping; j++) {
                value = constrain(mad.pFftMag[i + j], MIN_FFT_DB, MAX_FFT_DB);
                value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, maxHeightScaled); // fins a 89 // 169
                avg += value;
                if (value > max) {
                    max = value;
                }
            }
            if (i > 3 && i < (THE_PANEL_WIDTH / 5) && _1stBarValue < max) {
                _1stBarValue = max;
            }
            i += __binGrouping;
            value = avg / __binGrouping;
            for (uint8_t j = 0; j < __binGrouping - 1; j++) {
                _ThePanel.DrawBar(i + j - __binGrouping, max, false, __alterDraw); // j%2==0?false:true);
            }
        } else {
            _ThePanel.DrawBar(i, value);
            if (i > 3 && i < (THE_PANEL_WIDTH / 5) && _1stBarValue < value) {
                _1stBarValue = value;
            }
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

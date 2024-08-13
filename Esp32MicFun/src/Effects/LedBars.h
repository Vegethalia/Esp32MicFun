
void DrawLedBars(MsgAudio2Draw& mad)
{
    const auto numItems = _TheMapping.GetWidth();
    uint8_t maxBassValue = 0;
    int16_t value = 0;
    uint8_t maxHeight = min(16, BAR_HEIGHT); // 18
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

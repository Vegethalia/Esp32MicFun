
void DrawHorizSpectrogram(MsgAudio2Draw& mad)
{
    int16_t value = 0;

    //_TheLeds.fill_solid(CRGB(1,1,1));
    //   FastLED.clear();
    assert(BAR_HEIGHT > 1);
    // const auto numItems = _TheMapping.GetHeight();
    // uint8_t values[numItems];

    // for (uint16_t i = 0; i < numItems; i++) {
    //     value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
    //     value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, 255);
    //     values[i] = (uint8_t)value;
    // }
    _ThePanel.PushBar(mad.pDBs);

    // FastLED.show();
}

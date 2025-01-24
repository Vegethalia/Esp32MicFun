enum VertSpectrogramStyle : uint8_t {
    FIRE = 0,
    RUNING
};

void DrawVertSpectrogram(MsgAudio2Draw& mad, VertSpectrogramStyle style = VertSpectrogramStyle::FIRE)
{
    int16_t value = 0;

    static int8_t incValue = +1;
    static int8_t actualInc = 0;
    static const int8_t MAX_INC = 32;
    static std::vector<uint8_t> _TheRunningValues;

    if (_numFrames % 4 == 0) {
        actualInc += incValue;
        if (actualInc >= MAX_INC) {
            incValue = -1;
        } else if (actualInc == 0) {
            incValue = +1;
        }
    }

    if (_TheDesiredHue < 0) {
        _ThePanel.SetBaseHue((HSVHue::HUE_ORANGE - (MAX_INC / 3)) + actualInc);
    } else {
        _ThePanel.SetBaseHue((_TheDesiredHue - (MAX_INC / 3)) + actualInc);
    }
    uint8_t currentHue = _ThePanel.GetBaseHue();
    //_ThePanel.IncBaseHue();

    assert(THE_PANEL_WIDTH > 1);
    const auto numItems = _TheMapping.GetWidth();
    //uint8_t values[numItems];
    //memset8(values, 0, sizeof(values));

    _1stBarValue = 0;
    for (uint16_t i = 0; i < _TheMapping.GetWidth(); i++) {
        // value = constrain(mad.pFftMag[i], (int)MIN_FFT_DB, MAX_FFT_DB);
        // value = map(value, (int)MIN_FFT_DB, MAX_FFT_DB, 0, 255);
        // values[i] = (uint8_t)value;
        // if (i <= 5 && _1stBarValue < value) {
        //     _1stBarValue = value;
        // }
        if (_pianoMode && mad.pDBs[i] < 100) { //40% of the max value
            mad.pDBs[i] = 0;
        }
    }
    if (style == VertSpectrogramStyle::FIRE) {
        // if (_WithClock) { //jcs 2024-09-27 do not hide anything. bars can go UNDER the clock
        //     _ThePanel.PushLine(values, THE_PANEL_WIDTH - CLOCK_HORIZ_PIXELS, CLOCK_VERT_PIXELS - 1);
        // } else {
        _ThePanel.PushLine(mad.pDBs);
        // }
    } else {
        if (_TheRunningValues.size() == 0) {
            _TheRunningValues.resize(THE_PANEL_WIDTH * THE_PANEL_HEIGHT);
            memset8(_TheRunningValues.data(), 0, _TheRunningValues.size());
        }
        // desplacem totes les linies cap amunt
        for (uint16_t j = 0; j < THE_PANEL_HEIGHT - 1; j++) {
            memcpy(&_TheRunningValues[j * THE_PANEL_WIDTH], &_TheRunningValues[(j + 1) * THE_PANEL_WIDTH], THE_PANEL_WIDTH);
        }
        // copiem la nova linia
        memcpy(&_TheRunningValues[(THE_PANEL_HEIGHT - 1) * THE_PANEL_WIDTH], mad.pDBs, THE_PANEL_WIDTH);
        // mostrem les linies al panel
        //  i ara restaurem el buffer auxiliar en l'array de leds
        for (int i = 0; i < THE_PANEL_WIDTH; i++) {
            for (int j = 0; j < THE_PANEL_HEIGHT; j++) {
                uint8_t resta = _TheRunningValues[j * THE_PANEL_WIDTH + i] > (THE_PANEL_HEIGHT - j) * 2 ? (THE_PANEL_HEIGHT - j) * 2 : _TheRunningValues[j * THE_PANEL_WIDTH + i];
                _TheLeds[_TheMapping.XY(i, j)] += CHSV(currentHue, 255 - (THE_PANEL_HEIGHT - j) * 2,
                    (_TheRunningValues[j * THE_PANEL_WIDTH + i]) - resta);
                //                _TheLeds[_TheMapping.XY(i, j)].fadeToBlackBy((THE_PANEL_HEIGHT-j)*4);
            }
        }
    }
}

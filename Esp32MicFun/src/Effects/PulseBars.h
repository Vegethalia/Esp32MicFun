
void DrawPulses(MsgAudio2Draw& mad)
{
    static uint8_t* __pScreenMap = nullptr;
    static uint32_t* __pLastUpdateMillis = nullptr;
    //_ThePanel.IncBaseHue();

    if (__pScreenMap == nullptr) {
        __pScreenMap = new uint8_t[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
        memset8(__pScreenMap, 0, THE_PANEL_WIDTH * THE_PANEL_HEIGHT);
        __pLastUpdateMillis = new uint32_t[THE_PANEL_WIDTH];
        memset(__pLastUpdateMillis, 0, THE_PANEL_WIDTH * sizeof(uint32_t));
    }

    assert(THE_PANEL_WIDTH > 1);
    const auto numItems = _TheMapping.GetWidth();
    uint8_t values[numItems];
    int16_t value = 0;
    memset8(values, 0, sizeof(values));

    _1stBarValue = 0;
    uint32_t now = millis();
    for (int16_t i = 0; i < _TheMapping.GetWidth(); i++) {
        // value = constrain(mad.pFftMag[i], (int)MIN_FFT_DB, MAX_FFT_DB);
        // value = map(value, (int)MIN_FFT_DB, MAX_FFT_DB, 0, 255);
        value = mad.pDBs[i];

        int8_t minValue = 225 - i;
        uint8_t newValue = (uint8_t)(value < minValue ? 0 : 255); // all or nothing!
        bool doPulse = value > (200 - i);
        if (doPulse && (now - __pLastUpdateMillis[i]) > 600) {
            __pLastUpdateMillis[i] = now;
            values[i] = newValue;
        }
    }
    // std::string msg(Utils::string_format("MinMag=%d MaxMag=%d MinMap=%d MaxMap=%d", minMag, maxMag, minMap, maxMap));
    // _ThePubSub.publish(TOPIC_DEBUG, msg.c_str(), false);
    // actualitzem el nostre "screen" desplaçant tot cap amunt i inserint la nova línia per sota
    for (int i = 0; i < THE_PANEL_WIDTH; i++) {
        //deplacem tot cap a munt
        for (int j = 0; j < THE_PANEL_HEIGHT - 1; j++) {
            __pScreenMap[j * THE_PANEL_WIDTH + i] = __pScreenMap[(j + 1) * THE_PANEL_WIDTH + i] * 0.960;
        }
        //__pScreenMap[(THE_PANEL_HEIGHT - 1) * THE_PANEL_WIDTH + i] = values[i];

        //pintem la nova línia i si cal picar un "puls" pintem el "míssil" amb la cua degradada
        const int j = THE_PANEL_HEIGHT - 3;
        if (values[i] > 0) {
            __pScreenMap[j * THE_PANEL_WIDTH + i] = values[i];
            __pScreenMap[(j + 1) * THE_PANEL_WIDTH + i] = values[i] * 0.85;
        }
        __pScreenMap[(j + 2) * THE_PANEL_WIDTH + i] = values[i] * 0.70;
    }
    // for (int i = 0; i < THE_PANEL_WIDTH; i++) {
    //     const int j = THE_PANEL_HEIGHT - 3;
    //     if (values[i] > 0) {
    //         __pScreenMap[j * THE_PANEL_WIDTH + i] = values[i];
    //         __pScreenMap[(j + 1) * THE_PANEL_WIDTH + i] = values[i] * 0.85;
    //     }
    //     __pScreenMap[(j + 2) * THE_PANEL_WIDTH + i] = values[i] * 0.70;
    // }
    // pintem el nostre "screen" en el panel
    uint8_t currentHue = _ThePanel.GetBaseHue();
    for (int i = 0; i < THE_PANEL_WIDTH; i++) {
        for (int j = 0; j < THE_PANEL_HEIGHT; j++) {
            _TheLeds[_TheMapping.XY(i, j)] += CHSV(currentHue, 255, __pScreenMap[j * THE_PANEL_WIDTH + i]);
        }
    }
}

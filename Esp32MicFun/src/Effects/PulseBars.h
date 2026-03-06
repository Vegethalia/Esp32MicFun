
void DrawPulses(MsgAudio2Draw& mad)
{
    static uint8_t* __pScreenMap = nullptr;
    static uint32_t* __pLastUpdateMillis = nullptr;
    static uint16_t* __pLedIndexMap = nullptr;
    constexpr uint8_t PULSE_DECAY = 245;  // ~0.96 in Q8
    constexpr uint8_t TRAIL_85 = 218;     // ~0.85 in Q8
    constexpr uint8_t TRAIL_70 = 179;     // ~0.70 in Q8
    //_ThePanel.IncBaseHue();
    const uint16_t width = THE_PANEL_WIDTH;
    const uint16_t height = THE_PANEL_HEIGHT;
    const uint16_t totalPixels = width * height;

    if (__pScreenMap == nullptr) {
        __pScreenMap = new uint8_t[totalPixels];
        memset8(__pScreenMap, 0, totalPixels);
        __pLastUpdateMillis = new uint32_t[width];
        memset(__pLastUpdateMillis, 0, width * sizeof(uint32_t));
        __pLedIndexMap = new uint16_t[totalPixels];
        for (uint16_t j = 0; j < height; j++) {
            for (uint16_t i = 0; i < width; i++) {
                __pLedIndexMap[(j * width) + i] = _TheMapping.XY(i, j);
            }
        }
    }

    assert(THE_PANEL_WIDTH > 1);
    uint8_t values[THE_PANEL_WIDTH];
    int16_t value = 0;
    memset8(values, 0, sizeof(values));

    _1stBarValue = 0;
    uint32_t now = millis();
    for (uint16_t i = 0; i < width; i++) {
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
    for (uint16_t i = 0; i < width; i++) {
        //deplacem tot cap a munt
        for (uint16_t j = 0; j < height - 1; j++) {
            const uint16_t idx = (j * width) + i;
            __pScreenMap[idx] = (uint8_t)(((uint16_t)__pScreenMap[idx + width] * PULSE_DECAY) >> 8);
        }
        //__pScreenMap[(height - 1) * width + i] = values[i];

        //pintem la nova línia i si cal picar un "puls" pintem el "míssil" amb la cua degradada
        const uint16_t newLine = ((height - 3) * width) + i;
        if (values[i] > 0) {
            __pScreenMap[newLine] = values[i];
            __pScreenMap[newLine + width] = (uint8_t)(((uint16_t)values[i] * TRAIL_85) >> 8);
        }
        __pScreenMap[newLine + (2 * width)] = (uint8_t)(((uint16_t)values[i] * TRAIL_70) >> 8);
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
    for (uint16_t idx = 0; idx < totalPixels; idx++) {
        uint8_t v = __pScreenMap[idx];
        if (v) {
            _TheLeds[__pLedIndexMap[idx]] += CHSV(currentHue, 255, v);
        }
    }
}

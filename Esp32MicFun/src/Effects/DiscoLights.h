
void DrawDiscoLights(MsgAudio2Draw& mad)
{
    // 32-64-128-256-512  --> 192Hz 384Hz 768Hz 1536Hz 3072Hz
    const uint16_t bins[] = { 110 / HZ_PER_BIN, 240 / HZ_PER_BIN, 500 / HZ_PER_BIN, 1500 / HZ_PER_BIN, mad.sizeFftMagVector };
    const uint8_t max_circle = 6;
    const uint8_t totalLights = sizeof(bins) / sizeof(uint16_t);
    const HSVHue hues[totalLights] = { HSVHue::HUE_RED, HSVHue::HUE_BLUE, HSVHue::HUE_GREEN, HSVHue::HUE_PINK, HSVHue::HUE_YELLOW };
    const uint8_t intensityOff[totalLights] = { 40, 45, 40, 50, 50 };
    uint8_t dbRange = abs((MAX_FFT_DB - MIN_FFT_DB));
    const uint8_t dbs_x_circle = abs((MAX_FFT_DB - MIN_FFT_DB) / (int)(max_circle));
    uint16_t currentBin = 0;
    // uint8_t numLight = 0;

    // Calculate max BASS power among first bars
    // _1stBarValue = 0;
    // for (uint16_t i = 0; i < THE_PANEL_WIDTH / 5; i++) {
    //     auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
    //     value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
    //     if (_1stBarValue < value) {
    //         _1stBarValue = value;
    //     }
    // }

    for (uint8_t numLight = 0; numLight < totalLights; numLight++) {
        int8_t maxDb = -100;
        while (currentBin < bins[numLight]) {
            if (mad.pFftMag[currentBin] > maxDb) {
                maxDb = mad.pFftMag[currentBin];
            }
            currentBin++;
        }
        for (int8_t currDbs = MIN_FFT_DB, nCircle = 0; nCircle < max_circle; currDbs += dbs_x_circle, nCircle++) {
            if (currDbs > (MIN_FFT_DB + 5) && currDbs <= maxDb) {
                _ThePanel.DrawCircle((numLight * 12) + max_circle + 1, 17, nCircle, CHSV(hues[numLight], 255, (nCircle * 10) + 70)); // 255/6 circles= 42
            } else {
                _ThePanel.DrawCircle((numLight * 12) + max_circle + 1, 17, nCircle, CHSV(hues[numLight], 160, intensityOff[numLight])); // 255/6 circles= 42
            }
        }
    }
}

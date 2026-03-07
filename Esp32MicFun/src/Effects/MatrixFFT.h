
void DrawMatrixFFT(MsgAudio2Draw& mad)
{
    const uint8_t numLines = mad.sizeFftMagVector / THE_PANEL_WIDTH;
    uint8_t currentLine = THE_PANEL_HEIGHT - 1;
    uint16_t currentBin = 0;
    uint8_t x;

    int16_t theHue = HSVHue::HUE_PURPLE;
    if (_TheDesiredHue >= 0) {
        theHue = _TheDesiredHue;
    }

    for (uint8_t iLine = 0; iLine < numLines; iLine++) {
        uint16_t rowBase = currentLine * THE_PANEL_WIDTH;
        for (uint16_t xPix = 0; xPix < THE_PANEL_WIDTH; xPix++, currentBin++) {
            auto value = constrain(mad.pFftMag[currentBin], MIN_FFT_DB, MAX_FFT_DB);
            value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, 255);
            if (value < 5) { // black? el fem transparent :)
                continue;
            }

            if (iLine % 2 == 0) {
                x = xPix;
            } else {
                x = THE_PANEL_WIDTH - xPix - 1;
            }
            _TheLeds[LedIndexFlat(rowBase + x)] = CHSV(theHue + (value / 2), 255, (uint8_t)value);
        }
        currentLine--;
    }
}

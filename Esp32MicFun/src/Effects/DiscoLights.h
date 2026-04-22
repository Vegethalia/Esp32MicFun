
void DrawDiscoLights(MsgAudio2Draw& mad) {
  // Frequency bands (with HZ_PER_BIN = 6 Hz/bin, sizeFftMagVector = 512 → max ~3072 Hz):
  //  0 -   80 Hz : sub-bass    (kick drum fundamental)
  //  80 -  300 Hz: bass        (bass guitar, bass drum body)
  //  300 - 1000 Hz: low-mid   (guitar, vocals body)
  //  1000 - 2500 Hz: mid-high (presence, harmonics, snare)
  //  2500 Hz+    : treble      (hi-hats, cymbals, air)
  // Guard: cap bins[3] to sizeFftMagVector-1 to avoid out-of-bounds reads.
  const uint16_t maxBin = mad.sizeFftMagVector;
  const uint16_t bins[] = {
    80   / HZ_PER_BIN,
    300  / HZ_PER_BIN,
    1000 / HZ_PER_BIN,
    min((uint16_t)(2500 / HZ_PER_BIN), (uint16_t)(maxBin - 1u)),
    maxBin
  };
#if defined(PANEL_SIZE_96x54)
  const uint8_t max_circle = 9;
  const uint8_t yPos0 = (THE_PANEL_HEIGHT - (max_circle * 2)) + 2;
#else
  const uint8_t max_circle = 6;
  const uint8_t yPos0 = 17;
#endif
  const uint8_t totalLights = sizeof(bins) / sizeof(uint16_t);
  const HSVHue hues[totalLights] = {HSVHue::HUE_RED, HSVHue::HUE_ORANGE, HSVHue::HUE_GREEN, HSVHue::HUE_AQUA, HSVHue::HUE_PURPLE};
  const uint8_t intensityOff[totalLights] = {40, 40, 40, 45, 45};
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

  const uint8_t widthCircle = max_circle * 2;  // 12 pixels wide

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
        _ThePanel.DrawCircle((numLight * widthCircle) + max_circle + 1, yPos0, nCircle, CHSV(hues[numLight], 255, (nCircle * 10) + 70));  // 255/6 circles= 42
      } else {
        _ThePanel.DrawCircle((numLight * widthCircle) + max_circle + 1, yPos0, nCircle, CHSV(hues[numLight], 160, intensityOff[numLight]));  // 255/6 circles= 42
      }
    }
  }
}

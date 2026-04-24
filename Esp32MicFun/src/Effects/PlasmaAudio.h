
// Plasma Audio – sinusoidal colour-field, fully audio-reactive.
//
// Four overlapping sine waves create a classic "plasma" effect:
//   v1 = sin8( x·3  + t1 )   – horizontal, treble-driven
//   v2 = sin8( y·5  + t2 )   – vertical,   mid-driven   (hoisted per row)
//   v3 = sin8((x+y)·2 + t3 ) – diagonal,   bass-driven
//   v4 = sin8( dist·5 + t1+t3) – radial (concentric rings)
//   v  = (v1+v2+v3+v4) / 4  → hue offset
//
// All trig via FastLED sin8() (LUT). Radial distance via octagonal
// approximation (max+min/2, error ≤6%, imperceptible for plasma).
// Phases v1/v3 incremented per pixel (no multiply in inner loop).
// CPU estimate: ~150 K cycles/frame at 96×54.
// Memory: PlasmaData on heap – 4 bytes + 3 floats ≈ 16 bytes.
//
//   Bass    → pulse speed of radial + diagonal waves
//   Mid     → speed of vertical wave
//   Treble  → hue shift speed + horizontal wave speed
//   Volume  → overall brightness (50..140)

struct PlasmaData {
  uint8_t t1       = 0;
  uint8_t t2       = 0;
  uint8_t t3       = 0;
  uint8_t hueShift = 0;
  float   sBass    = 0.0f;
  float   sMid     = 0.0f;
  float   sTreble  = 0.0f;
};
PlasmaData* _plasmaData = nullptr;

void CleanupPlasma() {
  delete _plasmaData;
  _plasmaData = nullptr;
}

void DrawPlasma(MsgAudio2Draw& mad) {
  if (!_plasmaData) {
    _plasmaData = new PlasmaData();
  }

  // ---- 1. Per-band energy (normalised 0..1) ----
  const uint8_t BASS_END = 6, MID_END = 24;
  float bass = 0, mid = 0, treble = 0;
  for (uint8_t i = 0;        i < BASS_END;        i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END; i < MID_END;         i++) mid    += mad.pDBs[i];
  for (uint8_t i = MID_END;  i < THE_PANEL_WIDTH; i++) treble += mad.pDBs[i];
  bass   /= (float)BASS_END                    * 255.0f;
  mid    /= (float)(MID_END   - BASS_END)      * 255.0f;
  treble /= (float)(THE_PANEL_WIDTH - MID_END) * 255.0f;

  const float ALPHA = 0.10f;
  _plasmaData->sBass   += ALPHA * (bass   - _plasmaData->sBass);
  _plasmaData->sMid    += ALPHA * (mid    - _plasmaData->sMid);
  _plasmaData->sTreble += ALPHA * (treble - _plasmaData->sTreble);

  // ---- 2. Advance time counters ----
  _plasmaData->t1       += 3 + (uint8_t)(_plasmaData->sTreble * 10.0f);
  _plasmaData->t2       += 2 + (uint8_t)(_plasmaData->sMid    *  7.0f);
  _plasmaData->t3       += 1 + (uint8_t)(_plasmaData->sBass   *  8.0f);
  _plasmaData->hueShift += 1 + (uint8_t)(_plasmaData->sTreble *  5.0f);

  // ---- 3. Pre-frame constants ----
  const uint8_t volBright = (uint8_t)(50u + (uint16_t)_1stBarValue * 90u / 255u); // 50..140
  const uint8_t sat       = (uint8_t)(215u + (uint8_t)(_plasmaData->sBass * 35.0f));
  const uint8_t t3t1      = _plasmaData->t3 + _plasmaData->t1;

  const uint8_t W  = THE_PANEL_WIDTH;
  const uint8_t H  = THE_PANEL_HEIGHT;
  const int16_t cx = (int16_t)(W >> 1);
  const int16_t cy = (int16_t)(H >> 1);

  // ---- 4. Render ----
  uint16_t flatIdx = 0;
  for (uint8_t py = 0; py < H; py++) {
    // Per-row constants (hoisted out of inner loop)
    const int16_t dy   = (int16_t)py - cy;
    const uint8_t ady  = (uint8_t)(dy < 0 ? -dy : dy);
    const uint8_t v2   = sin8(py * 5u + _plasmaData->t2);
    const uint8_t diagBase = (uint8_t)(py * 2u + _plasmaData->t3);

    // Incremental phases: avoids px*3 and px*2 multiplications per pixel
    uint8_t phase1 = _plasmaData->t1;   // = sin8 arg for px=0: 0*3 + t1
    uint8_t phase3 = diagBase;           // = sin8 arg for px=0: (0+py)*2 + t3

    for (uint8_t px = 0; px < W; px++, flatIdx++, phase1 += 3, phase3 += 2) {
      const uint8_t v1 = sin8(phase1);
      const uint8_t v3 = sin8(phase3);

      // Octagonal distance approx: max(|dx|,|dy|) + min(|dx|,|dy|)>>1
      const int16_t dx  = (int16_t)px - cx;
      const uint8_t adx = (uint8_t)(dx < 0 ? -dx : dx);
      const uint8_t dist = (adx > ady) ? (uint8_t)(adx + (ady >> 1))
                                       : (uint8_t)(ady + (adx >> 1));
      const uint8_t v4  = sin8((uint8_t)(dist * 5u) + t3t1);

      const uint8_t v   = (uint8_t)(((uint16_t)v1 + v2 + v3 + v4) >> 2);
      const uint8_t hue = _plasmaData->hueShift + v;
      const uint8_t val = scale8(qadd8(v, 30), volBright);

      _TheLeds[LedIndexFlat(flatIdx)] = CHSV(hue, sat, val);
    }
  }
}

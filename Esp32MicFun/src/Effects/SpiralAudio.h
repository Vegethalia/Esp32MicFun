
// SpiralAudio – audio-reactive double-arm spinning spiral.
//
// Uses the same precomputed log-distance + angle LUTs as VortexTunnel.
// The spiral pattern combines angle and distance in a single sin8 call:
//   sin8( a*2 + d*4 - spiralPhase )
// giving two logarithmic spiral arms that ROTATE as spiralPhase advances.
// scale8(arm,arm) sharpens the result into thin glowing lines.
//
// Difference vs VortexTunnel: instead of rings zooming toward you,
// two coloured arms spin like a galaxy / yin-yang / double helix.
//
// Audio mapping:
//   Bass    → spin speed (1..21 advances/frame)
//   Treble  → hue-shift speed
//   Volume  → overall brightness (50..140)
//
// Memory: ~10.4 KB heap (2 × W×H uint8_t LUTs + 12 B state).
// CPU: identical to VortexTunnel inner loop.

struct SpiralAudioData {
  uint8_t distBuf[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
  uint8_t angleBuf[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
  uint8_t spiralPhase;
  uint8_t huePhase;
  float   sBass;
  float   sTreble;
};
SpiralAudioData* _spiralData = nullptr;

void CleanupSpiralAudio() {
  delete _spiralData;
  _spiralData = nullptr;
}

void DrawSpiralAudio(MsgAudio2Draw& mad) {
  const uint8_t W = THE_PANEL_WIDTH;
  const uint8_t H = THE_PANEL_HEIGHT;

  // ---- Init: precompute LUTs once ----
  if (!_spiralData) {
    _spiralData = new (std::nothrow) SpiralAudioData();
    if (!_spiralData) {
      log_e("SpiralAudio: alloc failed");
      ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true);
      return;
    }
    _spiralData->spiralPhase = 0;
    _spiralData->huePhase    = 0;
    _spiralData->sBass       = 0.0f;
    _spiralData->sTreble     = 0.0f;

    const float cx      = (float)(W >> 1);
    const float cy      = (float)(H >> 1);
    const float maxDist = sqrtf(cx * cx + cy * cy);

    for (uint8_t py = 0; py < H; py++) {
      for (uint8_t px = 0; px < W; px++) {
        const float    dx  = (float)px - cx;
        const float    dy  = (float)py - cy;
        const float    d   = sqrtf(dx * dx + dy * dy);
        const uint16_t idx = (uint16_t)py * W + px;

        // Log-scale distance: spiral arms are denser near centre
        const float t  = d / maxDist;
        const float ld = logf(1.0f + t * 31.0f) / logf(32.0f);
        _spiralData->distBuf[idx] = (uint8_t)(ld * 255.0f + 0.5f);

        // Angle 0..255
        const float ang = atan2f(dy, dx);
        _spiralData->angleBuf[idx] = (uint8_t)(128.0f + ang * (128.0f / 3.14159265f) + 0.5f);
      }
    }
  }

  // ---- 1. Per-band energy (normalised 0..1) ----
  const uint8_t BASS_END = 6;
  float bass = 0.0f, treble = 0.0f;
  for (uint8_t i = 0;        i < BASS_END; i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END; i < W;        i++) treble += mad.pDBs[i];
  bass   /= (float)BASS_END       * 255.0f;
  treble /= (float)(W - BASS_END) * 255.0f;

  const float ALPHA = 0.12f;
  _spiralData->sBass   += ALPHA * (bass   - _spiralData->sBass);
  _spiralData->sTreble += ALPHA * (treble - _spiralData->sTreble);

  // ---- 2. Advance phases ----
  // Bass: silent → barely moves (1/frame), full bass → fast spin (21/frame)
  _spiralData->spiralPhase += (uint8_t)(1u + (uint8_t)(_spiralData->sBass   * 20.0f));
  _spiralData->huePhase    += (uint8_t)(1u + (uint8_t)(_spiralData->sTreble *  6.0f));

  // ---- 3. Volume-driven brightness 25..140 ----
  const uint8_t volBright = (uint8_t)(25u + (uint16_t)_1stBarValue * 115u / 255u);

  // ---- 4. Render ----
  // a*2 → 2 equidistant arms (angle wraps twice over 360°)
  // d*4 → medium-tight spiral winding
  // spiralPhase increase → arms spin clockwise
  uint16_t flatIdx = 0;
  for (uint8_t py = 0; py < H; py++) {
    for (uint8_t px = 0; px < W; px++, flatIdx++) {
      const uint8_t d = _spiralData->distBuf[flatIdx];
      const uint8_t a = _spiralData->angleBuf[flatIdx];

      uint8_t arm = sin8((uint8_t)(a * 2u + d * 4u - _spiralData->spiralPhase));
      arm = scale8(arm, arm);  // sharpen to thin glowing lines

      const uint8_t val = scale8(qadd8(arm, 5u), volBright);
      if (val < 20u) continue;

      // Hue: angle drives rainbow arms, huePhase shifts the palette over time
      const uint8_t hue = a + _spiralData->huePhase + (d >> 2);
      const uint8_t sat = (d < 24u) ? (uint8_t)(180u + (uint16_t)d * 3u) : 230u;

      _TheLeds[LedIndexFlat(flatIdx)] = CHSV(hue, sat, val);
    }
  }
}

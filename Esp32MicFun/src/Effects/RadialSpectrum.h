
// RadialSpectrum – audio-reactive radial spectrum.
//
// THE_PANEL_WIDTH frequency bins are drawn as bars radiating from the panel
// center outward.  Each bar's length is proportional to its dB value.
// Colors shift continuously by angle (rainbow) plus a slow global hue drift.
//
// Design notes:
//   - cosine/sine LUT (float, computed once at first call, heap-allocated).
//   - persistence buffer (uint8_t[W*H]) faded ×0.84/frame for glowing trails.
//   - separate hue buffer (uint8_t[W*H]) so each pixel keeps its color.
//   - incremental radial draw: integer j from 1..length, float LUT multiply.
//
// Audio mapping:
//   Bass   → brightness boost (bars flash brighter on bass hits, +60 max)
//   Volume → base brightness (50..130)
//   Angle  → hue (full rainbow over 360°, slow global drift)
//
// Memory: ~10.8 KB heap (2× W×H uint8_t + 2× W float).
// CPU: ~3 K float ops + fade loop per frame at 96×54.

struct RadialSpectrumData {
  uint8_t persist[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
  uint8_t hueBuf [THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
  float   cosLUT [THE_PANEL_WIDTH];
  float   sinLUT [THE_PANEL_WIDTH];
  uint8_t baseHue;
  float   sBass;
};
RadialSpectrumData* _radialData = nullptr;

void CleanupRadialSpectrum() {
  delete _radialData;
  _radialData = nullptr;
}

void DrawRadialSpectrum(MsgAudio2Draw& mad) {
  const uint8_t W    = THE_PANEL_WIDTH;
  const uint8_t H    = THE_PANEL_HEIGHT;
  const int16_t cx   = (int16_t)(W >> 1);   // 48 for 96-wide
  const int16_t cy   = (int16_t)(H >> 1);   // 27 for 54-tall
  const uint8_t maxR = (uint8_t)(H >> 1);   // max bar length (27)

  // ---- Init (first call) ----
  if (!_radialData) {
    _radialData = new (std::nothrow) RadialSpectrumData();
    if (!_radialData) { log_e("RadialSpectrum: alloc failed"); ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true); return; }
    memset(_radialData->persist, 0, sizeof(_radialData->persist));
    memset(_radialData->hueBuf,  0, sizeof(_radialData->hueBuf));
    _radialData->baseHue = 0;
    _radialData->sBass   = 0.0f;
    // Precompute angle LUT
    for (uint8_t i = 0; i < W; i++) {
      const float angle = (float)i * (2.0f * 3.14159265f / (float)W);
      _radialData->cosLUT[i] = cosf(angle);
      _radialData->sinLUT[i] = sinf(angle);
    }
  }

  // ---- 1. Bass smoothing for brightness boost ----
  float bass = 0.0f;
  for (uint8_t i = 0; i < 6; i++) bass += mad.pDBs[i];
  bass /= (6.0f * 255.0f);
  _radialData->sBass += 0.15f * (bass - _radialData->sBass);

  // ---- 2. Slow global hue drift ----
  _radialData->baseHue++;

  // ---- 3. Fade persistence buffer (~×0.84 per frame) ----
  const uint16_t TOTAL = (uint16_t)W * H;
  for (uint16_t k = 0; k < TOTAL; k++) {
    uint8_t v = _radialData->persist[k];
    if (v > 4) _radialData->persist[k] = (uint8_t)(((uint16_t)v * 215u) >> 8);
    else if (v) _radialData->persist[k]--;
  }

  // ---- 4. Draw bars into persistence buffer ----
  const float cxf = (float)cx;
  const float cyf = (float)cy;

  for (uint8_t i = 0; i < W; i++) {
    const uint8_t db     = mad.pDBs[i];
    const uint8_t length = (uint8_t)(((uint16_t)db * maxR) >> 8);
    if (length == 0) continue;

    // Hue wraps full rainbow over the 360° ring, offset by global drift
    const uint8_t barHue = _radialData->baseHue + (uint8_t)((uint16_t)i * 255u / W);
    const float   cosI   = _radialData->cosLUT[i];
    const float   sinI   = _radialData->sinLUT[i];

    for (uint8_t j = 1; j <= length; j++) {
      const int16_t px = (int16_t)(cxf + cosI * (float)j + 0.5f);
      const int16_t py = (int16_t)(cyf + sinI * (float)j + 0.5f);
      if ((uint16_t)px >= W || (uint16_t)py >= H) continue;

      const uint16_t idx  = (uint16_t)py * W + (uint16_t)px;
      const uint16_t newV = (uint16_t)_radialData->persist[idx] + 80u;
      _radialData->persist[idx] = (newV > 255u) ? 255u : (uint8_t)newV;
      _radialData->hueBuf[idx]  = barHue;
    }
  }

  // ---- 5. Render ----
  // Bass adds up to +60 brightness units on top of the volume-driven base
  const uint8_t brightness = (uint8_t)(50u
    + (uint16_t)_1stBarValue * 80u / 255u
    + (uint8_t)(_radialData->sBass * 60.0f));
  for (uint8_t py = 0; py < H; py++) {
    for (uint8_t px = 0; px < W; px++) {
      const uint16_t idx = (uint16_t)py * W + (uint16_t)px;
      const uint8_t  br  = _radialData->persist[idx];
      if (br > 8) {
        const uint8_t hue = _radialData->hueBuf[idx];
        // bright cores go whiter, dim pixels stay saturated
        const uint8_t sat = (br > 200) ? 160 : 255;
        _TheLeds[_TheMapping.XY(px, py)] = CHSV(hue, sat, scale8(br, brightness));
      }
    }
  }
}


// Clifford Strange Attractor – audio-reactive fractal nebula.
//
// Coloring: velocity-direction hue.
//   Each pixel stores the direction of the orbit when it was last visited.
//   Different lobes flow in different directions → each lobe gets its own hue.
//   Bright cores → white-hot (low saturation); medium density → deep color.
//   Memory: heap-allocated only while style is active (~10.4 KB).

struct FractalAudioData {
  uint8_t persist[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];  // brightness
  uint8_t hueBuf [THE_PANEL_WIDTH * THE_PANEL_HEIGHT];  // velocity-hue
  float   fx      = 0.5f,  fy      = 0.3f;
  uint8_t baseHue = 80;
  float   sBass   = 0.0f,  sMid    = 0.0f,  sHiMid  = 0.0f;
  float   sVolFrac = 0.7f;
};
FractalAudioData* _fractalData = nullptr;

void CleanupFractalAudio() {
  delete _fractalData;
  _fractalData = nullptr;
}

void DrawFractalAudio(MsgAudio2Draw& mad) {
  if (!_fractalData) {
    _fractalData = new (std::nothrow) FractalAudioData();
    if (!_fractalData) { log_e("FractalAudio: alloc failed"); ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true); return; }
    memset(_fractalData->persist, 0, sizeof(_fractalData->persist));
    memset(_fractalData->hueBuf,  0, sizeof(_fractalData->hueBuf));
  }

  // ---- 1. Per-band energy ----
  const uint8_t BASS_END  =  6;
  const uint8_t MID_END   = 24;
  const uint8_t HIMID_END = 60;
  float bass = 0, mid = 0, himid = 0, treble = 0;
  for (uint8_t i = 0;          i < BASS_END;        i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END;   i < MID_END;         i++) mid    += mad.pDBs[i];
  for (uint8_t i = MID_END;    i < HIMID_END;        i++) himid  += mad.pDBs[i];
  for (uint8_t i = HIMID_END;  i < THE_PANEL_WIDTH;  i++) treble += mad.pDBs[i];
  bass   /= (float)(BASS_END)                    * 255.0f;
  mid    /= (float)(MID_END   - BASS_END)        * 255.0f;
  himid  /= (float)(HIMID_END - MID_END)         * 255.0f;
  treble /= (float)(THE_PANEL_WIDTH - HIMID_END) * 255.0f;
  const float ALPHA = 0.12f;
  _fractalData->sBass  += ALPHA * (bass  - _fractalData->sBass);
  _fractalData->sMid   += ALPHA * (mid   - _fractalData->sMid);
  _fractalData->sHiMid += ALPHA * (himid - _fractalData->sHiMid);

  // ---- 2. Attractor parameters ----
  float a = -1.4f - _fractalData->sBass  * 0.6f;
  float b =  1.6f + _fractalData->sMid   * 0.7f;
  float c =  1.0f + _fractalData->sHiMid * 0.5f;
  float d =  0.7f - _fractalData->sMid   * 0.5f;

  // ---- 3. Global hue drift + volume scale ----
  _fractalData->baseHue  += 1 + (uint8_t)(treble * 5.0f);
  _fractalData->sVolFrac += 0.04f * (0.3f + (_1stBarValue / 255.0f) * 0.7f - _fractalData->sVolFrac);

  // ---- 4. Fade brightness buffer (~×0.85/frame) ----
  const uint16_t TOTAL_PX = THE_PANEL_WIDTH * THE_PANEL_HEIGHT;
  for (uint16_t i = 0; i < TOTAL_PX; i++) {
    if (_fractalData->persist[i] > 4)  _fractalData->persist[i] = (uint8_t)((_fractalData->persist[i] * 218u) >> 8);
    else if (_fractalData->persist[i]) _fractalData->persist[i]--;
  }

  // ---- 5. Iterate attractor ----
  const uint16_t iters  = 220 + _1stBarValue;
  const float    xScale = (THE_PANEL_WIDTH  - 1) * 0.2f * _fractalData->sVolFrac;
  const float    yScale = (THE_PANEL_HEIGHT - 1) * 0.2f * _fractalData->sVolFrac;
  const float    xOff   = THE_PANEL_WIDTH  * 0.5f;
  const float    yOff   = THE_PANEL_HEIGHT * 0.5f + 5.0f;

  for (uint16_t i = 0; i < iters; i++) {
    const float nx = sinf(a * _fractalData->fy) + c * cosf(a * _fractalData->fx);
    const float ny = sinf(b * _fractalData->fx) + d * cosf(b * _fractalData->fy);
    const uint8_t velHue = _fractalData->baseHue
                         + (uint8_t)((int16_t)((nx - _fractalData->fx) * 28.0f)
                         +           (int16_t)((ny - _fractalData->fy) * 20.0f));
    _fractalData->fx = nx; _fractalData->fy = ny;
    const int16_t px = (int16_t)(_fractalData->fx * xScale + xOff + 0.5f);
    const int16_t py = (int16_t)(_fractalData->fy * yScale + yOff + 0.5f);
    if ((uint16_t)px < THE_PANEL_WIDTH && (uint16_t)py < THE_PANEL_HEIGHT) {
      const uint16_t idx = (uint16_t)py * THE_PANEL_WIDTH + (uint16_t)px;
      const uint16_t br = _fractalData->persist[idx] + 38u;
      _fractalData->persist[idx] = br > 255u ? 255u : (uint8_t)br;
      _fractalData->hueBuf[idx] = (uint8_t)(_fractalData->hueBuf[idx]
                  + (int16_t)((int16_t)velHue - (int16_t)_fractalData->hueBuf[idx]) / 8);
    }
  }

  // ---- 6. Render: velocity-hue + brightness-driven saturation ----
  for (uint8_t py = 0; py < THE_PANEL_HEIGHT; py++) {
    for (uint8_t px = 0; px < THE_PANEL_WIDTH; px++) {
      const uint16_t idx = (uint16_t)py * THE_PANEL_WIDTH + px;
      const uint8_t  br  = _fractalData->persist[idx];
      if (br > 8) {
        const uint8_t hue = _fractalData->hueBuf[idx];
        const uint8_t sat = (br > 200) ? 70
                          : (br > 120) ? 170
                          :              235;
        _TheLeds[_TheMapping.XY(px, py)] = CHSV(hue, sat, br);
      }
    }
  }
}

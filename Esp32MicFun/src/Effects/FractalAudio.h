
// Clifford Strange Attractor – audio-reactive fractal nebula.
//
// How it works:
//   Each frame the Clifford recurrence  x' = sin(a·y) + c·cos(a·x)
//                                       y' = sin(b·x) + d·cos(b·y)
//   is iterated several hundred times.  Each visited pixel is brightened in
//   a persistence buffer that fades by ~15% per frame, producing a glowing
//   nebula with trailing light.  The four parameters are gently driven by
//   audio bands so the fractal morphs and breathes with the music.
//
//   Bass   → parameter a  (warps the main lobes)
//   Mid    → parameters b & d  (stretches secondary orbit)
//   Hi-mid → parameter c  (widens lateral spread)
//   Treble → hue rotation speed
//   Volume → iteration count (louder = denser, brighter fractal)
//
//   Memory: 1 static uint8_t[W×H] in BSS (5184 B for 96×54) – not stack.

void DrawFractalAudio(MsgAudio2Draw& mad) {
  static uint8_t persist[THE_PANEL_WIDTH * THE_PANEL_HEIGHT] = {};  // glow buffer (BSS)
  static float   fx = 0.5f, fy = 0.3f;                             // running attractor point
  static uint8_t baseHue   = 80;
  static float   sBass     = 0.0f, sMid = 0.0f, sHiMid = 0.0f;    // smoothed bands
  static float   sVolFrac  = 0.7f;   // smooth scale factor (0.3 silent → 1.0 loud)

  // ---- 1. Per-band energy (pDBs is 0-255, THE_PANEL_WIDTH bins) ----
  const uint8_t BASS_END  =  6;
  const uint8_t MID_END   = 24;
  const uint8_t HIMID_END = 60;

  float bass = 0, mid = 0, himid = 0, treble = 0;
  for (uint8_t i = 0;          i < BASS_END;       i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END;   i < MID_END;        i++) mid    += mad.pDBs[i];
  for (uint8_t i = MID_END;    i < HIMID_END;      i++) himid  += mad.pDBs[i];
  for (uint8_t i = HIMID_END;  i < THE_PANEL_WIDTH; i++) treble += mad.pDBs[i];
  bass   /= (float)(BASS_END)                    * 255.0f;
  mid    /= (float)(MID_END   - BASS_END)        * 255.0f;
  himid  /= (float)(HIMID_END - MID_END)         * 255.0f;
  treble /= (float)(THE_PANEL_WIDTH - HIMID_END) * 255.0f;

  // Smooth parameter changes to avoid jittery shape transitions
  const float ALPHA = 0.12f;
  sBass  += ALPHA * (bass  - sBass);
  sMid   += ALPHA * (mid   - sMid);
  sHiMid += ALPHA * (himid - sHiMid);

  // ---- 2. Attractor parameters driven by audio ----
  // Base values (-1.4, 1.6, 1.0, 0.7) → 4-lobed symmetric galaxy at rest.
  // The Clifford recurrence is always bounded (sin/cos bounded), so the
  // attractor stays within ≈ [-2.5, 2.5]² for the full audio-modulated range.
  float a = -1.4f - sBass  * 0.6f;   // bass deepens the main warp
  float b =  1.6f + sMid   * 0.7f;   // mid stretches secondary warp
  float c =  1.0f + sHiMid * 0.5f;   // hi-mid widens lateral spread
  float d =  0.7f - sMid   * 0.5f;   // mid compresses vertical orbit

  // ---- 3. Hue rotates slowly; treble accelerates it ----
  baseHue += 1 + (uint8_t)(treble * 5.0f);

  // ---- 3b. Volume scale: shrinks when silent, expands when loud ----
  sVolFrac += 0.04f * (0.3f + (_1stBarValue / 255.0f) * 0.7f - sVolFrac);

  // ---- 4. Fade glow buffer — integer fast path (≈×0.85 per frame) ----
  const uint16_t TOTAL_PX = THE_PANEL_WIDTH * THE_PANEL_HEIGHT;
  for (uint16_t i = 0; i < TOTAL_PX; i++) {
    if (persist[i] > 4)  persist[i] = (uint8_t)((persist[i] * 218u) >> 8);
    else if (persist[i]) persist[i]--;
  }

  // ---- 5. Iterate attractor and accumulate brightness ----
  // Louder audio → more iterations → denser, brighter fractal
  const uint16_t iters  = 220 + _1stBarValue;            // 220-475 per frame
  // Scale: sVolFrac maps attractor [-2.5,2.5] to a fraction of the panel.
  // Centre at panel centre (+5px Y offset); shrinks when silent, fills when loud.
  const float xScale = (THE_PANEL_WIDTH  - 1) * 0.2f * sVolFrac;
  const float yScale = (THE_PANEL_HEIGHT - 1) * 0.2f * sVolFrac;
  const float xOff   = THE_PANEL_WIDTH  * 0.5f;
  const float yOff   = THE_PANEL_HEIGHT * 0.5f + 5.0f;  // centre 5px lower

  for (uint16_t i = 0; i < iters; i++) {
    float nx = sinf(a * fy) + c * cosf(a * fx);
    float ny = sinf(b * fx) + d * cosf(b * fy);
    fx = nx;
    fy = ny;
    // Projection: centred on panel, scaled by volume
    int16_t px = (int16_t)(fx * xScale + xOff + 0.5f);
    int16_t py = (int16_t)(fy * yScale + yOff + 0.5f);
    // Cast to uint16_t: negative values wrap to large numbers, failing the range check
    if ((uint16_t)px < THE_PANEL_WIDTH && (uint16_t)py < THE_PANEL_HEIGHT) {
      uint16_t idx = (uint16_t)py * THE_PANEL_WIDTH + (uint16_t)px;
      uint16_t br  = persist[idx] + 38u;
      persist[idx] = br > 255u ? 255u : (uint8_t)br;
    }
  }

  // ---- 6. Render glow buffer → LEDs with rainbow-nebula colouring ----
  // Hue varies across the panel: rotating base + position offset → natural gradient.
  // Bright cores shift towards white (lower saturation) for a glowing star look.
  for (uint8_t py = 0; py < THE_PANEL_HEIGHT; py++) {
    for (uint8_t px = 0; px < THE_PANEL_WIDTH; px++) {
      uint8_t br = persist[(uint16_t)py * THE_PANEL_WIDTH + px];
      if (br > 8) {
        uint8_t hue = baseHue + px + py * 2;
        uint8_t sat = br > 160 ? 160 : 220;  // hot core fades towards white
        _TheLeds[_TheMapping.XY(px, py)] = CHSV(hue, sat, br);
      }
    }
  }
}

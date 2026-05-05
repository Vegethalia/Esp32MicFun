
// VortexTunnel – audio-reactive hypnotic zoom tunnel.
//
// Two uint8_t LUTs (log-distance + angle from centre) are computed ONCE at first
// call and kept on the heap. Per-frame work is just 2 byte-LUT lookups + sin8()
// + a few adds per pixel — NO floating-point in the inner loop.
//
// Visual: concentric rings appear to zoom outward from the vanishing point,
// simulating flight into a wormhole. The distance LUT uses a log-scale so rings
// are denser near the centre (real tunnel perspective foreshortening). The angle
// LUT drives the hue so each sector has its own colour; a d>>1 spiral term gives
// a subtle twist to the tunnel walls.
//
// Audio mapping:
//   Bass    → zoom speed (zoomPhase advance 3..17 per frame)
//   Mid     → rotation speed of the colour wheel (rotPhase)
//   Treble  → hue-shift speed (huePhase)
//   Volume  → overall brightness (50..140)
//
// Memory: ~10.4 KB heap (2 × W×H uint8_t LUTs + 20 B state).
// CPU: ~5184 × (2 LUT reads + sin8 + scale8 + qadd8) — lighter than PlasmaAudio.

struct VortexTunnelData {
  uint8_t distBuf[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];   // log-distance 0..255 from centre
  uint8_t angleBuf[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];  // angle 0..255 (right=128, wraps CW)
  uint8_t zoomPhase;  // ring animation phase (increasing → rings fly outward)
  uint8_t rotPhase;   // colour-wheel rotation
  uint8_t huePhase;   // global hue shift
  float   sBass;
  float   sMid;
  float   sTreble;
};
VortexTunnelData* _vortexData = nullptr;

void CleanupVortexTunnel() {
  delete _vortexData;
  _vortexData = nullptr;
}

void DrawVortexTunnel(MsgAudio2Draw& mad) {
  const uint8_t W = THE_PANEL_WIDTH;
  const uint8_t H = THE_PANEL_HEIGHT;

  // ---- Init: precompute LUTs once (float math only here) ----
  if (!_vortexData) {
    _vortexData = new (std::nothrow) VortexTunnelData();
    if (!_vortexData) {
      log_e("VortexTunnel: alloc failed");
      ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true);
      return;
    }
    _vortexData->zoomPhase = 0;
    _vortexData->rotPhase  = 0;
    _vortexData->huePhase  = 0;
    _vortexData->sBass     = 0.0f;
    _vortexData->sMid      = 0.0f;
    _vortexData->sTreble   = 0.0f;

    const float cx      = (float)(W >> 1);
    const float cy      = (float)(H >> 1);
    const float maxDist = sqrtf(cx * cx + cy * cy);

    for (uint8_t py = 0; py < H; py++) {
      for (uint8_t px = 0; px < W; px++) {
        const float    dx  = (float)px - cx;
        const float    dy  = (float)py - cy;
        const float    d   = sqrtf(dx * dx + dy * dy);
        const uint16_t idx = (uint16_t)py * W + px;

        // Log-scale distance: dense rings near centre → real tunnel perspective.
        // t in [0..1]; log(1 + t*31)/log(32) maps t linearly to a compressed scale.
        const float t   = d / maxDist;
        const float ld  = logf(1.0f + t * 31.0f) / logf(32.0f);
        _vortexData->distBuf[idx] = (uint8_t)(ld * 255.0f + 0.5f);

        // Angle: atan2 maps -π..π → 0..255
        const float ang = atan2f(dy, dx);
        _vortexData->angleBuf[idx] = (uint8_t)(128.0f + ang * (128.0f / 3.14159265f) + 0.5f);
      }
    }
  }

  // ---- 1. Per-band energy (normalised 0..1) ----
  const uint8_t BASS_END = 6, MID_END = 24;
  float bass = 0.0f, mid = 0.0f, treble = 0.0f;
  for (uint8_t i = 0;        i < BASS_END; i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END; i < MID_END;  i++) mid    += mad.pDBs[i];
  for (uint8_t i = MID_END;  i < W;        i++) treble += mad.pDBs[i];
  bass   /= (float)BASS_END               * 255.0f;
  mid    /= (float)(MID_END - BASS_END)   * 255.0f;
  treble /= (float)(W - MID_END)          * 255.0f;

  const float ALPHA = 0.12f;
  _vortexData->sBass   += ALPHA * (bass   - _vortexData->sBass);
  _vortexData->sMid    += ALPHA * (mid    - _vortexData->sMid);
  _vortexData->sTreble += ALPHA * (treble - _vortexData->sTreble);

  // ---- 2. Advance animation phases ----
  // Bass: silent → very slow drift (1/frame), full bass → fast rush (21/frame)
  _vortexData->zoomPhase += (uint8_t)(1u + (uint8_t)(_vortexData->sBass   * 20.0f));
  _vortexData->rotPhase  += (uint8_t)(1u + (uint8_t)(_vortexData->sMid    *  5.0f));
  _vortexData->huePhase  += (uint8_t)(1u + (uint8_t)(_vortexData->sTreble *  6.0f));

  // ---- 3. Volume-driven brightness 25..140 ----
  const uint8_t volBright = (uint8_t)(25u + (uint16_t)_1stBarValue * 115u / 255u);

  // ---- 4. Render (inner-loop: integer only) ----
  uint16_t flatIdx = 0;
  for (uint8_t py = 0; py < H; py++) {
    for (uint8_t px = 0; px < W; px++, flatIdx++) {
      const uint8_t d = _vortexData->distBuf[flatIdx];
      const uint8_t a = _vortexData->angleBuf[flatIdx];

      // Rings: ×8 → ~8 rings on screen simultaneously.
      // scale8(v,v) ≈ v²/255: sharpens peaks into thin bright lines on dark bg.
      uint8_t ring = sin8((uint8_t)(d * 8u - _vortexData->zoomPhase));
      ring = scale8(ring, ring);

      // Hue: sector angle + global phases + d>>1 for a subtle wall spiral
      const uint8_t hue = a + _vortexData->rotPhase + _vortexData->huePhase + (d >> 1);

      // Brightness: small floor so dark gaps stay dark, scaled by volume
      const uint8_t val = scale8(qadd8(ring, 5u), volBright);

      // Skip very dark pixels → clean black background between rings
      if (val < 10u) continue;

      // Saturation: white-hot core bleeds to full colour outward
      const uint8_t sat = (d < 24u) ? (uint8_t)(180u + (uint16_t)d * 3u) : 230u;

      _TheLeds[LedIndexFlat(flatIdx)] = CHSV(hue, sat, val);
    }
  }
}

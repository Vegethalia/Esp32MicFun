
// Mandelbrot Set - audio-reactive progressive fractal.
//
// Render area: rows [MANDEL_Y_START .. THE_PANEL_HEIGHT-1]
// (clock area at the top is left untouched; ~24% fewer pixels to compute)
//
// Progressive: MANDEL_ROWS_PER_FRAME rows recomputed each frame ->
// full MANDEL_HEIGHT refresh every ~2-3 frames.
//
// Rotation: the complex plane is rotated each frame. cos/sin computed once
// per frame; per-pixel cost is just two float additions instead of one.
//
//   Bass    -> zoom + rotation speed
//   Mid     -> slow horizontal drift
//   Treble  -> hue rotation speed
//   Volume  -> brightness

static constexpr uint8_t MANDEL_Y_START = CLOCK_VERT_PIXELS + 3;
static constexpr uint8_t MANDEL_HEIGHT = THE_PANEL_HEIGHT - MANDEL_Y_START;
static constexpr uint8_t MANDEL_ROWS_PER_FRAME = 20;  // 18;
static constexpr uint8_t MANDEL_MAX_ITER = 27;        // 30;        // 24;        // reduced: interior pixels do 40% fewer iters
static constexpr float MANDEL_CX = -0.7269f;
static constexpr float MANDEL_CY = 0.1889f;
static constexpr float MANDEL_ZOOM_WIDE = 0.028f;
static constexpr float MANDEL_ZOOM_TIGHT = 0.0010f;  // 0.0015f;

struct MandelbrotData {
  uint8_t escBuf[THE_PANEL_WIDTH * MANDEL_HEIGHT];  // escape-time buffer
  uint8_t baseHue = 0;
  uint8_t curRow  = 0;
  float   zoom    = MANDEL_ZOOM_WIDE;
  float   panX    = 0.0f;
  float   angle   = 0.0f;
  float   sBass   = 0.0f, sMid = 0.0f, sTreble = 0.0f;
};
MandelbrotData* _mandelbrotData = nullptr;

void CleanupMandelbrot() {
  delete _mandelbrotData;
  _mandelbrotData = nullptr;
}

void DrawMandelbrot(MsgAudio2Draw& mad) {
  if (!_mandelbrotData) {
    _mandelbrotData = new (std::nothrow) MandelbrotData();
    if (!_mandelbrotData) { log_e("Mandelbrot: alloc failed"); ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true); return; }
    memset(_mandelbrotData->escBuf, 0, sizeof(_mandelbrotData->escBuf));
  }

  // ---- 1. Audio bands (normalised 0..1) ----
  const uint8_t BASS_END = 6, MID_END = 24;
  float bass = 0, mid = 0, treble = 0;
  for (uint8_t i = 0; i < BASS_END; i++) bass += mad.pDBs[i];
  for (uint8_t i = BASS_END; i < MID_END; i++) mid += mad.pDBs[i];
  for (uint8_t i = MID_END; i < THE_PANEL_WIDTH; i++) treble += mad.pDBs[i];
  bass /= (float)BASS_END * 255.0f;
  mid /= (float)(MID_END - BASS_END) * 255.0f;
  treble /= (float)(THE_PANEL_WIDTH - MID_END) * 255.0f;
  const float A = 0.05f;
  _mandelbrotData->sBass   += A * (bass   - _mandelbrotData->sBass);
  _mandelbrotData->sMid    += A * (mid    - _mandelbrotData->sMid);
  _mandelbrotData->sTreble += A * (treble - _mandelbrotData->sTreble);

  // ---- 2. View update (once per frame) ----
  const float targetZoom = MANDEL_ZOOM_WIDE - _mandelbrotData->sBass * (MANDEL_ZOOM_WIDE - MANDEL_ZOOM_TIGHT);
  _mandelbrotData->zoom += 0.04f * (targetZoom - _mandelbrotData->zoom);
  _mandelbrotData->panX += (_mandelbrotData->sMid - 0.25f) * 0.000015f;
  if (_mandelbrotData->panX >  0.04f) _mandelbrotData->panX =  0.04f;
  if (_mandelbrotData->panX < -0.04f) _mandelbrotData->panX = -0.04f;
  _mandelbrotData->angle += 0.004f + _mandelbrotData->sBass * 0.04f;
  _mandelbrotData->baseHue += 1u + (uint8_t)(_mandelbrotData->sTreble * 5.0f);

  // Rotation basis vectors (computed once per frame, not per pixel)
  const float cosA = cosf(_mandelbrotData->angle) * _mandelbrotData->zoom;
  const float sinA = sinf(_mandelbrotData->angle) * _mandelbrotData->zoom;
  // Half-panel offsets for centering
  const float halfW = THE_PANEL_WIDTH * 0.5f;
  const float halfH = THE_PANEL_HEIGHT * 0.5f;

  // ---- 3. Progressive computation: MANDEL_ROWS_PER_FRAME rows ----
  {
    const float sy0 = (float)(MANDEL_Y_START + _mandelbrotData->curRow) - halfH;
    float rowCr = MANDEL_CX + _mandelbrotData->panX - halfW * cosA - sy0 * sinA;
    float rowCi = MANDEL_CY - halfW * sinA + sy0 * cosA;

    for (uint8_t r = 0; r < MANDEL_ROWS_PER_FRAME; r++, rowCr -= sinA, rowCi += cosA) {
      float cr = rowCr;
      float ci = rowCi;
      uint8_t* rowPtr = _mandelbrotData->escBuf + (uint16_t)_mandelbrotData->curRow * THE_PANEL_WIDTH;

      for (uint8_t px = 0; px < THE_PANEL_WIDTH; px++, cr += cosA, ci += sinA) {
        // Cardioid / period-2 bulb check: analytically detect main in-set bodies.
        // Costs ~6 muls but skips all 24 iterations for those pixels.
        const float ci2 = ci * ci;
        const float crq = cr - 0.25f;
        const float q = crq * crq + ci2;
        if (q * (q + crq) <= 0.25f * ci2 ||
            (cr + 1.0f) * (cr + 1.0f) + ci2 <= 0.0625f) {
          rowPtr[px] = 0;  // in main body, skip iteration
          continue;
        }
        float zr = 0.0f, zi = 0.0f;
        uint8_t iter = 0;
        // Brent cycle detection: detects periodic orbits of ANY period.
        // Saves all remaining iterations for pixels inside higher-period bulbs
        // and mini-Mandelbrot sets (not covered by the cardioid/bulb check above).
        // Cost: 2 fabs + 2 compares per iteration. Period doubles: 1,2,4,8,16.
        float zr_chk = 0.0f, zi_chk = 0.0f;
        uint8_t period = 1, cnt = 0;
        while (iter < MANDEL_MAX_ITER) {
          if (zr > 2.0f || zr < -2.0f || zi > 2.0f || zi < -2.0f) break;
          const float zr2 = zr * zr, zi2 = zi * zi;
          if (zr2 + zi2 > 4.0f) break;
          zi = 2.0f * zr * zi + ci;
          zr = zr2 - zi2 + cr;
          ++iter;
          if (fabsf(zr - zr_chk) < 1e-5f && fabsf(zi - zi_chk) < 1e-5f) {
            iter = MANDEL_MAX_ITER; break;
          }
          if (++cnt == period) {
            zr_chk = zr; zi_chk = zi; cnt = 0;
            if (period < 16) period <<= 1;
          }
        }
        rowPtr[px] = (iter >= MANDEL_MAX_ITER) ? 0u : (uint8_t)(iter + 1u);
      }
      _mandelbrotData->curRow = (_mandelbrotData->curRow + 1u) % MANDEL_HEIGHT;
    }
  }

  // ---- 4. Build color LUT then render ----
  const uint8_t volBright = qadd8(_1stBarValue, 40);
  CRGB lut[MANDEL_MAX_ITER + 1];
  lut[0] = CHSV(_mandelbrotData->baseHue, 220, scale8(18, volBright));
  for (uint8_t i = 1; i <= MANDEL_MAX_ITER; i++) {
    const uint8_t bri = scale8((i < 12u) ? (uint8_t)(i * 18u) : 216u, volBright);
    lut[i] = CHSV(_mandelbrotData->baseHue + i * 5u, 230, bri);
  }

  for (uint8_t py = 0; py < MANDEL_HEIGHT; py++) {
    const uint8_t panelY = MANDEL_Y_START + py;
    const uint8_t* rowPtr = _mandelbrotData->escBuf + (uint16_t)py * THE_PANEL_WIDTH;
    for (uint8_t px = 0; px < THE_PANEL_WIDTH; px++) {
      if (rowPtr[px] <= 2) continue;  // interior + zona limítrofe: deixa veure el fons
      _TheLeds[_TheMapping.XY(px, panelY)] = lut[rowPtr[px]];
    }
  }
}
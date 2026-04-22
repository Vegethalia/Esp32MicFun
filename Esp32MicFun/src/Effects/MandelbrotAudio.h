
// Mandelbrot Set - audio-reactive progressive fractal.
//
// Render area: rows [MANDEL_Y_START .. THE_PANEL_HEIGHT-1]
// (clock area at the top is left untouched; ~24% fewer pixels to compute)
//
// Progressive: MANDEL_ROWS_PER_FRAME rows recomputed each frame ->
// full MANDEL_HEIGHT refresh every ~2-3 frames.
//
//   Bass    -> zoom  (loud bass zooms into the spiral)
//   Mid     -> slow horizontal drift
//   Treble  -> hue rotation speed
//   Volume  -> brightness

static constexpr uint8_t MANDEL_Y_START        = CLOCK_VERT_PIXELS + 2;
static constexpr uint8_t MANDEL_HEIGHT         = THE_PANEL_HEIGHT - MANDEL_Y_START;
static constexpr uint8_t MANDEL_ROWS_PER_FRAME = 18;
static constexpr uint8_t MANDEL_MAX_ITER        = 40;
static constexpr float   MANDEL_CX              = -0.7269f;
static constexpr float   MANDEL_CY              =  0.1889f;
static constexpr float   MANDEL_ZOOM_WIDE       =  0.028f;
static constexpr float   MANDEL_ZOOM_TIGHT      =  0.0015f;

void DrawMandelbrot(MsgAudio2Draw& mad) {
  static uint8_t escBuf[THE_PANEL_WIDTH * MANDEL_HEIGHT] = {};
  static uint8_t baseHue = 0;
  static uint8_t curRow  = 0;   // relative to MANDEL_Y_START
  static float   zoom    = MANDEL_ZOOM_WIDE;
  static float   panX    = 0.0f;
  static float   sBass   = 0.0f, sMid = 0.0f, sTreble = 0.0f;

  // ---- 1. Audio bands (normalised 0..1) ----
  const uint8_t BASS_END = 6, MID_END = 24;
  float bass = 0, mid = 0, treble = 0;
  for (uint8_t i = 0;        i < BASS_END;        i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END; i < MID_END;         i++) mid    += mad.pDBs[i];
  for (uint8_t i = MID_END;  i < THE_PANEL_WIDTH; i++) treble += mad.pDBs[i];
  bass   /= (float)BASS_END                    * 255.0f;
  mid    /= (float)(MID_END - BASS_END)        * 255.0f;
  treble /= (float)(THE_PANEL_WIDTH - MID_END) * 255.0f;
  const float A = 0.05f;
  sBass   += A * (bass   - sBass);
  sMid    += A * (mid    - sMid);
  sTreble += A * (treble - sTreble);

  // ---- 2. View update (once per frame) ----
  const float targetZoom = MANDEL_ZOOM_WIDE - sBass * (MANDEL_ZOOM_WIDE - MANDEL_ZOOM_TIGHT);
  zoom += 0.04f * (targetZoom - zoom);
  panX += (sMid - 0.25f) * 0.000015f;
  if (panX >  0.04f) panX =  0.04f;
  if (panX < -0.04f) panX = -0.04f;
  baseHue += 1u + (uint8_t)(sTreble * 5.0f);

  // ---- 3. Progressive computation: MANDEL_ROWS_PER_FRAME rows ----
  // Map panel pixel (px, MANDEL_Y_START+curRow) -> Mandelbrot coordinate (cr, ci)
  const float crLeft = MANDEL_CX + panX - THE_PANEL_WIDTH  * 0.5f * zoom;
  const float ciTop  = MANDEL_CY        - THE_PANEL_HEIGHT * 0.5f * zoom;

  for (uint8_t r = 0; r < MANDEL_ROWS_PER_FRAME; r++) {
    const uint8_t panelRow = MANDEL_Y_START + curRow;
    const float   ci       = ciTop + panelRow * zoom;
    float         cr       = crLeft;
    uint8_t*      rowPtr   = escBuf + (uint16_t)curRow * THE_PANEL_WIDTH;

    for (uint8_t px = 0; px < THE_PANEL_WIDTH; px++, cr += zoom) {
      float zr = 0.0f, zi = 0.0f;
      uint8_t iter = 0;
      while (iter < MANDEL_MAX_ITER) {
        const float zr2 = zr * zr, zi2 = zi * zi;
        if (zr2 + zi2 > 4.0f) break;
        zi = 2.0f * zr * zi + ci;
        zr = zr2 - zi2 + cr;
        ++iter;
      }
      rowPtr[px] = (iter >= MANDEL_MAX_ITER) ? 0u : (uint8_t)(iter + 1u);
    }
    curRow = (curRow + 1u) % MANDEL_HEIGHT;
  }

  // ---- 4. Build color LUT (41 entries) then render ----
  // One CHSV->RGB conversion per possible iter value instead of one per pixel.
  const uint8_t volBright = qadd8(_1stBarValue, 40);
  CRGB lut[MANDEL_MAX_ITER + 1];
  lut[0] = CHSV(baseHue, 220, scale8(18, volBright));  // in-set (dark)
  for (uint8_t i = 1; i <= MANDEL_MAX_ITER; i++) {
    const uint8_t bri = scale8((i < 12u) ? (uint8_t)(i * 18u) : 216u, volBright);
    lut[i] = CHSV(baseHue + i * 5u, 230, bri);
  }

  for (uint8_t py = 0; py < MANDEL_HEIGHT; py++) {
    const uint8_t  panelY = MANDEL_Y_START + py;
    const uint8_t* rowPtr = escBuf + (uint16_t)py * THE_PANEL_WIDTH;
    for (uint8_t px = 0; px < THE_PANEL_WIDTH; px++) {
      _TheLeds[_TheMapping.XY(px, panelY)] = lut[rowPtr[px]];
    }
  }
}
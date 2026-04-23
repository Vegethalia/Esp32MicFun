
// Lissajous Parametric – audio-reactive with persistence trails.
//
// Demo mode: smooth Lissajous evolution from circle to complex curves.
//   Phase 0: circle  (1:1)
//   Phase 1: figure-8 (1:2)
//   Phase 2: trefoil  (1:3)
//   Phase 3: knot     (2:3)
//   Phase 4: complex  (3:4)
// Each phase target is approached with IIR smoothing; audio adds subtle
// modulation on top so the shape "breathes" with the music.
//
// Rotation: slow spin, speed driven by bass.
// Memory:   heap-allocated only while active (~10 KB; no curve arrays needed).

#if defined(PANEL_SIZE_96x54)
#define MOVING_PARAMETRIC_STEPS 360
constexpr float PARAM_XAMP = (THE_PANEL_WIDTH / 2.0) - 0.5;
constexpr float PARAM_YAMP = (THE_PANEL_HEIGHT / 2.0) - 0.5;
#else
#define MOVING_PARAMETRIC_STEPS 252
constexpr float PARAM_XAMP = (THE_PANEL_WIDTH / 2.0) - 0.5;
constexpr float PARAM_YAMP = (THE_PANEL_HEIGHT / 2.0) - 0.5;
#endif
constexpr float MOVING_STEP   = (2.0 * M_PI) / MOVING_PARAMETRIC_STEPS;
constexpr float MOVING_NEG_PI = -(float)M_PI;

// Demo evolution: Lissajous phase targets (xFreq : yFreq)
#define MAX_DEMO_PHASES 5
static constexpr float DEMO_PHASE_X[MAX_DEMO_PHASES] = {1.0f, 1.0f, 1.0f, 2.0f, 3.0f};
static constexpr float DEMO_PHASE_Y[MAX_DEMO_PHASES] = {1.0f, 2.0f, 3.0f, 3.0f, 4.0f};
static constexpr uint16_t DEMO_FRAMES_PER_PHASE = 250;

struct DrawParametricData {
  uint16_t curveOffset = 0;  // animation phase: all steps drawn, offset crawls each frame
  bool initialized = false;
  float delta = 0.0f;
  uint8_t hue = 0;
  float xMag = 1.0f;
  float yMag = 1.0f;
  int16_t textPos = THE_PANEL_WIDTH - 2;
  int16_t inc = -1;
  bool alreadyDrawedText = false;
  uint8_t currentPhase = 0;
  // Audio-reactive state
  float sBass = 0.0f, sMid = 0.0f, sHiMid = 0.0f, sTreble = 0.0f;
  float rotAngle = 0.0f;
  uint8_t baseHue = 0;
  uint32_t animFrame = 0;
  uint8_t fastFadeFrames = 0;
  // Persistence trail buffers (zeroed at init)
  uint8_t persist[THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
  uint8_t hueBuf [THE_PANEL_WIDTH * THE_PANEL_HEIGHT];
};

DrawParametricData* __dpd = nullptr;

void CleanupParametric() {
  if (__dpd) {
    delete __dpd;
    __dpd = nullptr;
  }
}

void DrawParametric(MsgAudio2Draw& mad) {
  if (!__dpd) {
    __dpd = new DrawParametricData();
  }

  // ---- 1. Per-band energy ----
  const uint8_t BASS_END  =  6;
  const uint8_t MID_END   = 24;
  const uint8_t HIMID_END = 60;
  float bass = 0, mid = 0, himid = 0, treble = 0;
  for (uint8_t i = 0;         i < BASS_END;        i++) bass   += mad.pDBs[i];
  for (uint8_t i = BASS_END;  i < MID_END;         i++) mid    += mad.pDBs[i];
  for (uint8_t i = MID_END;   i < HIMID_END;       i++) himid  += mad.pDBs[i];
  for (uint8_t i = HIMID_END; i < THE_PANEL_WIDTH; i++) treble += mad.pDBs[i];
  bass   /= (float)(BASS_END)                    * 255.0f;
  mid    /= (float)(MID_END   - BASS_END)        * 255.0f;
  himid  /= (float)(HIMID_END - MID_END)         * 255.0f;
  treble /= (float)(THE_PANEL_WIDTH - HIMID_END) * 255.0f;
  const float ALPHA = 0.10f;
  __dpd->sBass   += ALPHA * (bass   - __dpd->sBass);
  __dpd->sMid    += ALPHA * (mid    - __dpd->sMid);
  __dpd->sHiMid  += ALPHA * (himid  - __dpd->sHiMid);
  __dpd->sTreble += ALPHA * (treble - __dpd->sTreble);

  bool newPhase = false;

  if (!__dpd->initialized) {
    memset(__dpd->persist, 0, sizeof(__dpd->persist));
    memset(__dpd->hueBuf,  0, sizeof(__dpd->hueBuf));
    __dpd->initialized = true;
  }

  // ---- 2. Demo mode: smooth Lissajous evolution ----
  if (_DemoMode && (_Connected2Wifi || _TheFrameNumber > 200)) {
    if (_DemoModeFrame == 0) {
      __dpd->currentPhase = 0;
      __dpd->xMag = DEMO_PHASE_X[0];
      __dpd->yMag = DEMO_PHASE_Y[0];
      __dpd->alreadyDrawedText = false;
    }

    // Advance phase every DEMO_FRAMES_PER_PHASE frames
    if (_DemoModeFrame > 0 && (_DemoModeFrame % DEMO_FRAMES_PER_PHASE) == 0) {
      __dpd->currentPhase++;
      newPhase = true;
      __dpd->alreadyDrawedText = false;
      __dpd->fastFadeFrames = 12;
      if (__dpd->currentPhase >= MAX_DEMO_PHASES) {
        _DemoMode = false;
        _u8g2.setFont(u8g2_font_princess_tr);
        log_d("DeMo MoDe FINISH");
        if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
          _UpdateCurrentNow = true;
        }
      }
    }

    if (_DemoMode) {
      // IIR toward target frequencies: ~95% in 100 frames (~2s at 48fps)
      const uint8_t p = __dpd->currentPhase;
      __dpd->xMag += 0.03f * (DEMO_PHASE_X[p] - __dpd->xMag);
      __dpd->yMag += 0.03f * (DEMO_PHASE_Y[p] - __dpd->yMag);
      _DemoModeFrame++;
    }
  }

  // ---- 3. Curve frequencies for this frame ----
  // Demo:   IIR base + subtle audio modulation (shape stays recognisable)
  // Normal: full audio-reactive drift with non-integer ratios (always morphing)
  float displayX, displayY;
  if (_DemoMode) {
    displayX = __dpd->xMag + __dpd->sBass * 0.35f;
    displayY = __dpd->yMag + __dpd->sMid  * 0.35f;
  } else {
    // Autonomous drift ensures varied ratios even in silence; audio amplifies the swings.
    // Bass modulates X freq, treble modulates Y freq → different instruments change different axes.
    const float af = (float)__dpd->animFrame * 0.001f;
    const float targetX = 1.0f + 0.7f * fabsf(sinf(af * 0.4f))        + __dpd->sBass   * 3.0f;
    const float targetY = 2.0f + 0.7f * fabsf(cosf(af * 0.3f + 1.2f)) + __dpd->sTreble * 5.0f;
    __dpd->xMag += 0.05f * (targetX - __dpd->xMag);
    __dpd->yMag += 0.05f * (targetY - __dpd->yMag);
    displayX = __dpd->xMag;
    displayY = __dpd->yMag;
  }

  const float cx = PARAM_XAMP;
  const float cy = PARAM_YAMP;

  // ---- 4. Rotation ----
  __dpd->rotAngle += 0.012f + __dpd->sBass * 0.06f;
  const float cosR = cosf(__dpd->rotAngle);
  const float sinR = sinf(__dpd->rotAngle);

  // ---- 5. Hue drift with treble ----
  __dpd->baseHue += 1 + (uint8_t)(__dpd->sTreble * 2.0f);

  // ---- 6. Fade persistence trails ----
  const uint8_t fadeCoeff = 210u;
  const uint16_t TOTAL_PX = THE_PANEL_WIDTH * THE_PANEL_HEIGHT;
  if (__dpd->fastFadeFrames > 0) {
    for (uint16_t i = 0; i < TOTAL_PX; i++) __dpd->persist[i] = (__dpd->persist[i] * 154u) >> 8;
    __dpd->fastFadeFrames--;
  } else {
    for (uint16_t i = 0; i < TOTAL_PX; i++) {
      if (__dpd->persist[i] > 4)  __dpd->persist[i] = (__dpd->persist[i] * fadeCoeff) >> 8;
      else if (__dpd->persist[i]) __dpd->persist[i]--;
    }
  }

  // ---- 7+8. Compute + draw in one pass: sin/cos inline, no intermediate arrays ----
  const uint8_t addBright = 35 + (_1stBarValue >> 2);
  const uint16_t yMinClip = 0u;

  // Advance crawl offset every 2 frames
  if ((__dpd->animFrame % 2) == 0) {
    __dpd->curveOffset = (__dpd->curveOffset + 1) % MOVING_PARAMETRIC_STEPS;
  }

  uint8_t theHue = __dpd->baseHue;
  for (uint16_t i = 0; i < MOVING_PARAMETRIC_STEPS; i++) {
    const uint16_t step = (__dpd->curveOffset + i) % MOVING_PARAMETRIC_STEPS;
    const float t = MOVING_NEG_PI + step * (float)MOVING_STEP;
    // Lissajous: x = A·cos(fx·t),  y = B·sin(fy·t + δ)   [cosf = sinf + HALF_PI]
    const float rawX = PARAM_XAMP    * cosf(displayX * t) + PARAM_XAMP;
    const float rawY = PARAM_YAMP * sinf(displayY * t + __dpd->delta) + PARAM_YAMP;
    // Rotate around curve centre
    const float dx = rawX - cx;
    const float dy = rawY - cy;
    const int16_t rx = (int16_t)(dx * cosR - dy * sinR + cx + 0.5f);
    const int16_t ry = (int16_t)(dx * sinR + dy * cosR + cy + 0.5f);
    if ((uint16_t)rx < THE_PANEL_WIDTH && (uint16_t)ry >= yMinClip && (uint16_t)ry < THE_PANEL_HEIGHT) {
      const uint16_t idx = (uint16_t)ry * THE_PANEL_WIDTH + (uint16_t)rx;
      const uint16_t br = __dpd->persist[idx] + addBright;
      __dpd->persist[idx] = (br > 255u) ? 255u : (uint8_t)br;
      __dpd->hueBuf[idx] = theHue;
    }
#if defined(PANEL_SIZE_96x54)
    theHue++;
#else
    theHue += 4;
#endif
  }
  __dpd->delta += (float)MOVING_STEP * (1.0f + __dpd->sTreble * 1.5f);
  if (__dpd->delta >= TWO_PI) __dpd->delta -= TWO_PI;
  __dpd->animFrame++;

  // ---- 9. Render from persistence buffer ----
  for (uint8_t py2 = 0; py2 < THE_PANEL_HEIGHT; py2++) {
    for (uint8_t px2 = 0; px2 < THE_PANEL_WIDTH; px2++) {
      const uint16_t idx = (uint16_t)py2 * THE_PANEL_WIDTH + px2;
      const uint8_t br = __dpd->persist[idx];
      if (br > 8) {
        const uint8_t sat = (br > 220) ? 160 : 235;
        _TheLeds[LedIndexXY(px2, py2)] = CHSV(__dpd->hueBuf[idx], sat, br);
      } else {
        _TheLeds[LedIndexXY(px2, py2)] = CRGB::Black;
      }
    }
  }

  // ---- 10. Text overlay (status messages, demo mode only) ----
  if (_DemoMode) {
  std::string test;
  if (__dpd->currentPhase == 0 && !_Connected2Wifi && !__dpd->alreadyDrawedText) {
    _u8g2.setFont(u8g2_font_oskool_tr);
    test = "Iniciant FlipaLeds...";
    _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
    __dpd->alreadyDrawedText = true;
  } else if (__dpd->currentPhase < (MAX_DEMO_PHASES - 2) && _Connected2Wifi && !__dpd->alreadyDrawedText) {
    _u8g2.clearBuffer();
    test = "Connectat a la wiFi!";
    _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
    __dpd->alreadyDrawedText = true;
  } else if (__dpd->currentPhase >= (MAX_DEMO_PHASES - 2)) {
    if (_Connected2Wifi && !__dpd->alreadyDrawedText) {
      _u8g2.clearBuffer();
      test = Utils::string_format("IP=[%s]", WiFi.localIP().toString().c_str());
      _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
      __dpd->alreadyDrawedText = true;
    } else if (!_Connected2Wifi && newPhase) {
      _u8g2.clearBuffer();
      test = Utils::string_format("Sense WiFi...");
      _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
    }
  }

  const uint8_t textBright = std::max((uint8_t)100, _1stBarValue);
#if defined(PANEL_SIZE_96x54)
  _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 5, 6, __dpd->textPos, THE_PANEL_HEIGHT - 16, __dpd->hue, textBright);
#else
  _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 2, 3, __dpd->textPos, 16, __dpd->hue, textBright);
#endif

  __dpd->textPos += __dpd->inc;
  if (__dpd->textPos < (-THE_PANEL_WIDTH * 2)) {
    __dpd->inc = 1;
  } else if (__dpd->textPos > THE_PANEL_WIDTH) {
    __dpd->inc = (-1);
  }
  __dpd->hue++;
  }  // end demo-only text overlay

  // Free heap once demo sequence ends (won't restart until reset).
  // LISSAJOUS_AUDIO manages its own lifetime via CleanupParametric() called from DrawerTask.
  if (!_DemoMode && __dpd && _TheDrawStyle != DRAW_STYLE::LISSAJOUS_AUDIO) {
    delete __dpd;
    __dpd = nullptr;
  }
}

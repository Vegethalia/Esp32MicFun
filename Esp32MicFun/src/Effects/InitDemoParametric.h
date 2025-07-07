
#if defined(PANEL_SIZE_96x48)
#define MOVING_PARAMETRIC_POINTS 360 //quants punts dibuixem
#define MOVING_PARAMETRIC_STEPS 360 // quants steps els fem rotar
constexpr float PARAM_XAMP = (THE_PANEL_WIDTH / 2.0) - 0.5;
constexpr float PARAM_YAMP = (THE_PANEL_HEIGHT / 2.0) - 0.5;
#else  // #define PANEL_SIZE_64x32
#define MOVING_PARAMETRIC_POINTS 64
#define MOVING_PARAMETRIC_STEPS 252
constexpr float PARAM_XAMP = (THE_PANEL_WIDTH / 2.0) - 0.5;
constexpr float PARAM_YAMP = (THE_PANEL_HEIGHT / 2.0) - 0.5;
#endif
constexpr float MOVING_STEP = (2.0 * M_PI) / MOVING_PARAMETRIC_STEPS;  // 2*PI / 252

struct ParametricCurve {
  uint16_t initialPoints[MOVING_PARAMETRIC_POINTS];
  float xCoord[MOVING_PARAMETRIC_STEPS];
  float yCoord[MOVING_PARAMETRIC_STEPS];
};

// ParametricCurve __2to3PiHalf = {
//     //{ 0, 25, 50, 75, 100, 125, 150, 175, 200, 225 },
//     { 0, 31, 63, 94, 126, 157, 189, 220 },
//     //{ 0, 43, 85, 127, 169, 211 },
//     { 0, 0.045, 0.18, 0.403, 0.715, 1.112, 1.593, 2.155, 2.795, 3.509, 4.293, 5.143, 6.054, 7.021, 8.039, 9.101, 10.202, 11.336, 12.496, 13.676, 14.868, 16.067, 17.266, 18.457, 19.635, 20.793, 21.923, 23.02, 24.078, 25.09, 26.051, 26.955, 27.798, 28.575, 29.281, 29.912, 30.465, 30.937, 31.325, 31.626, 31.84, 31.965, 31.999, 31.944, 31.8, 31.566, 31.245, 30.838, 30.348, 29.777, 29.129, 28.407, 27.615, 26.758, 25.84, 24.867, 23.844, 22.777, 21.672, 20.535, 19.373, 18.191, 16.998, 15.798, 14.6, 13.41, 12.234, 11.079, 9.952, 8.859, 7.807, 6.8, 5.845, 4.947, 4.111, 3.342, 2.645, 2.022, 1.478, 1.015, 0.637, 0.345, 0.142, 0.027, 0.002, 0.067, 0.222, 0.466, 0.796, 1.213, 1.712, 2.292, 2.948, 3.679, 4.478, 5.342, 6.266, 7.245, 8.273, 9.345, 10.454, 11.594, 12.759, 13.942, 15.137, 16.336, 17.534, 18.723, 19.897, 21.049, 22.172, 23.261, 24.309, 25.31, 26.259, 27.15, 27.978, 28.739, 29.429, 30.043, 30.578, 31.031, 31.4, 31.682, 31.876, 31.98, 31.995, 31.92, 31.755, 31.502, 31.161, 30.736, 30.227, 29.639, 28.973, 28.235, 27.428, 26.557, 25.627, 24.642, 23.609, 22.533, 21.42, 20.277, 19.109, 17.925, 16.729, 15.529, 14.332, 13.145, 11.973, 10.824, 9.704, 8.62, 7.577, 6.581, 5.638, 4.754, 3.933, 3.18, 2.498, 1.893, 1.367, 0.923, 0.564, 0.292, 0.108, 0.014, 0.009, 0.094, 0.269, 0.532, 0.882, 1.317, 1.835, 2.432, 3.106, 3.852, 4.666, 5.544, 6.481, 7.472, 8.51, 9.59, 10.707, 11.853, 13.023, 14.209, 15.405, 16.605, 17.802, 18.988, 20.157, 21.303, 22.419, 23.499, 24.537, 25.527, 26.464, 27.341, 28.155, 28.9, 29.573, 30.17, 30.687, 31.121, 31.471, 31.733, 31.907, 31.991, 31.986, 31.891, 31.706, 31.433, 31.073, 30.629, 30.102, 29.496, 28.814, 28.06, 27.238, 26.353, 25.41, 24.414, 23.371, 22.286, 21.166, 20.017, 18.845, 17.657, 16.46, 15.26, 14.065, 12.88, 11.713, 10.57, 9.458, 8.382, 7.349, 6.365, 5.435, 4.564, 3.758, 3.02, 2.356, 1.768, 1.26, 0.835, 0.496, 0.243, 0.079, 0.005 },
//     { 8, 8.375, 8.749, 9.121, 9.49, 9.856, 10.216, 10.572, 10.921, 11.262, 11.596, 11.92, 12.235, 12.539, 12.832, 13.112, 13.38, 13.635, 13.875, 14.101, 14.311, 14.506, 14.684, 14.846, 14.99, 15.117, 15.227, 15.318, 15.391, 15.445, 15.481, 15.498, 15.497, 15.476, 15.437, 15.38, 15.304, 15.21, 15.097, 14.967, 14.82, 14.655, 14.474, 14.277, 14.064, 13.836, 13.593, 13.336, 13.066, 12.783, 12.489, 12.183, 11.866, 11.54, 11.205, 10.862, 10.512, 10.156, 9.794, 9.428, 9.058, 8.686, 8.312, 7.937, 7.562, 7.189, 6.817, 6.448, 6.083, 5.723, 5.369, 5.021, 4.681, 4.349, 4.026, 3.713, 3.411, 3.12, 2.842, 2.576, 2.324, 2.086, 1.863, 1.655, 1.463, 1.288, 1.129, 0.987, 0.863, 0.757, 0.669, 0.599, 0.547, 0.515, 0.501, 0.505, 0.529, 0.571, 0.632, 0.711, 0.808, 0.923, 1.056, 1.207, 1.374, 1.558, 1.758, 1.974, 2.204, 2.449, 2.708, 2.981, 3.266, 3.562, 3.87, 4.188, 4.515, 4.852, 5.196, 5.547, 5.904, 6.267, 6.634, 7.004, 7.377, 7.751, 8.126, 8.501, 8.874, 9.245, 9.613, 9.977, 10.337, 10.69, 11.036, 11.375, 11.706, 12.027, 12.338, 12.639, 12.927, 13.204, 13.467, 13.717, 13.953, 14.173, 14.378, 14.568, 14.74, 14.896, 15.035, 15.156, 15.259, 15.345, 15.411, 15.459, 15.489, 15.5, 15.492, 15.465, 15.42, 15.356, 15.274, 15.174, 15.055, 14.92, 14.766, 14.596, 14.409, 14.207, 13.989, 13.756, 13.508, 13.247, 12.972, 12.685, 12.387, 12.077, 11.758, 11.429, 11.091, 10.745, 10.393, 10.035, 9.672, 9.304, 8.933, 8.56, 8.186, 7.811, 7.436, 7.063, 6.693, 6.325, 5.962, 5.604, 5.251, 4.906, 4.568, 4.239, 3.92, 3.61, 3.312, 3.025, 2.751, 2.49, 2.242, 2.009, 1.791, 1.589, 1.402, 1.232, 1.079, 0.943, 0.825, 0.725, 0.643, 0.579, 0.534, 0.508, 5.00, 0.511, 0.541, 0.589, 0.656, 0.741, 0.845, 0.966, 1.105, 1.261, 1.434, 1.623, 1.829, 2.05, 2.285, 2.535, 2.799, 3.075, 3.364, 3.664, 3.976, 4.297, 4.628, 4.967, 5.313, 5.667, 6.026, 6.39, 6.758, 7.129, 7.503, 7.877 }
// };

#define MAX_DEMO_PHASES 5
struct DemoModeParams {
  uint8_t numPhases;
  uint8_t currentPhase;
  uint16_t phaseChangeAtFrame[MAX_DEMO_PHASES];
  float phaseMagsx[MAX_DEMO_PHASES];
  float phaseMagsy[MAX_DEMO_PHASES];
};

struct DrawParametricData {
  ParametricCurve TheCurrentCurve;
  DemoModeParams TheDemoParams;
  bool initialized = false;
  // const uint16_t s_numCoords = 252;
  uint16_t frameNum = 0;
  float steps[MOVING_PARAMETRIC_STEPS];
  float delta = 0.0f;
  uint8_t hue = 0;
  float xMag = 1.0;
  float yMag = 1.0;
  bool reachingMag = false;
  bool rotating = false;
  uint32_t shiftCount = 0;
  int16_t textPos = THE_PANEL_WIDTH - 2;
  int16_t inc = -1;
  bool alreadyDrawedText = false;
};

DrawParametricData* __dpd = nullptr;
void DrawParametric(MsgAudio2Draw& mad) {
  // static ParametricCurve s_TheCurrentCurve;
  // static DemoModeParams s_TheDemoParams;
  // static bool s_initialized = false;
  // static const uint16_t s_numCoords = 252;
  // static uint16_t s_frameNum = 0;
  // static float s_steps[s_numCoords];
  // static float s_delta = 0.0f;
  // // static float s_deltaRatio = 0.01f;
  // // static float s_deltaRatioTotal = 1.0f;
  // static uint8_t s_hue = 0;
  // static float xMag = 1.0;
  // static float yMag = 1.0;
  // static bool reachingMag = false;
  // static bool rotating = false;
  // static uint32_t shiftCount = 0;
  // static int16_t textPos = THE_PANEL_WIDTH - 2;
  // static int16_t inc = -1;

  if (!__dpd) {
    __dpd = new DrawParametricData();
  }

  bool newPhase = false;
  // static bool alreadyDrawedText = false;
  // FastLED.clear();

  if (!__dpd->initialized) {
    float every = (float)MOVING_PARAMETRIC_STEPS / (float)MOVING_PARAMETRIC_POINTS;
    for (uint16_t i = 0; i < MOVING_PARAMETRIC_POINTS; i++) {
      __dpd->TheCurrentCurve.initialPoints[i] = (uint16_t)(every * i);
    }
    double menysPI = -PI;
    for (uint16_t i = 0; i < MOVING_PARAMETRIC_STEPS; i++) {
      __dpd->steps[i] = menysPI + (i * MOVING_STEP);  // 0.025=2Pi/252. 252 son el num d'steps
                                                      // s_TheCurrentCurve.xCoord[i] = 31.1f * sin(2 * s_steps[i] + HALF_PI) + 32.0f;
    }
    __dpd->initialized = true;
  }

  if ((_DemoMode && _Connected2Wifi) || (_TheFrameNumber > 200)) {
    if (_DemoModeFrame == 0) {
      __dpd->TheDemoParams.currentPhase = 0;
      __dpd->TheDemoParams.numPhases = 3;
      __dpd->TheDemoParams.phaseMagsx[0] = 1.0;
      __dpd->TheDemoParams.phaseMagsx[1] = 1.0;
      __dpd->TheDemoParams.phaseMagsx[2] = 1.0;
      __dpd->TheDemoParams.phaseMagsx[3] = 2.0;
      __dpd->TheDemoParams.phaseMagsx[4] = 3.0;
      __dpd->TheDemoParams.phaseMagsy[0] = 1.0;
      __dpd->TheDemoParams.phaseMagsy[1] = 2.0;
      __dpd->TheDemoParams.phaseMagsy[2] = 3.0;
      __dpd->TheDemoParams.phaseMagsy[3] = 3.0;
      __dpd->TheDemoParams.phaseMagsy[4] = 4.0;

      __dpd->xMag = __dpd->TheDemoParams.phaseMagsx[0];
      __dpd->yMag = __dpd->TheDemoParams.phaseMagsy[0];
      __dpd->reachingMag = false;
      __dpd->rotating = true;
    }

    if (__dpd->reachingMag) {
      if (__dpd->xMag < __dpd->TheDemoParams.phaseMagsx[__dpd->TheDemoParams.currentPhase]) {
        __dpd->xMag += 0.01;
      }
      if (__dpd->yMag < __dpd->TheDemoParams.phaseMagsy[__dpd->TheDemoParams.currentPhase]) {
        __dpd->yMag += 0.01;
      }
      if (__dpd->xMag >= __dpd->TheDemoParams.phaseMagsx[__dpd->TheDemoParams.currentPhase] && __dpd->yMag >= __dpd->TheDemoParams.phaseMagsy[__dpd->TheDemoParams.currentPhase]) {
        __dpd->reachingMag = false;
        __dpd->rotating = true;
      }
    }
    if (__dpd->rotating) {
      if (_DemoModeFrame >= ((uint32_t)(__dpd->TheDemoParams.currentPhase + 1) * 250)) {
        __dpd->TheDemoParams.currentPhase++;
        newPhase = true;
        __dpd->alreadyDrawedText = false;
        if (__dpd->TheDemoParams.currentPhase < MAX_DEMO_PHASES) {
          __dpd->rotating = false;
          __dpd->reachingMag = true;
        } else {
          // _DemoModeFrame = 0;
          // s_TheDemoParams.currentPhase = 0;
          // xMag = s_TheDemoParams.phaseMagsx[0];
          // yMag = s_TheDemoParams.phaseMagsy[0];
          _DemoMode = false;
          _u8g2.setFont(u8g2_font_princess_tr);
          log_d("DeMo MoDe FINISH");
          if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
            _UpdateCurrentNow = true;
          }
        }
      }
      _DemoModeFrame++;
    }
  }

  if (__dpd->frameNum == 0) {
    /* x = 31.1*sin(3*step+pi/2)+32
       y = 15.1*sin(2*step)+16*/
    for (uint16_t i = 0; i < MOVING_PARAMETRIC_STEPS; i++) {
      __dpd->TheCurrentCurve.xCoord[i] = (PARAM_XAMP)*sin(__dpd->xMag * __dpd->steps[i] + HALF_PI) + PARAM_XAMP;
      __dpd->TheCurrentCurve.yCoord[i] = (PARAM_YAMP)*sin(__dpd->yMag * __dpd->steps[i] + __dpd->delta) + PARAM_YAMP;
    }
    __dpd->delta += MOVING_STEP;
    if (__dpd->delta >= TWO_PI) {
      __dpd->delta = 0.0f;
    }
    // shiftCount = 0;
    //  if (s_deltaRatioTotal > 6.0 || s_deltaRatioTotal < 1.0) {
    //      s_deltaRatio = (-s_deltaRatio);
    //  }
    //  s_deltaRatioTotal += s_deltaRatio;
  }
  uint8_t intensity = std::max((uint8_t)(100), _1stBarValue);

  for (uint16_t i = 0; i < MOVING_PARAMETRIC_POINTS; i++) {
    if (__dpd->rotating && (_DemoModeFrame % 2) == 0) {
      __dpd->TheCurrentCurve.initialPoints[i] = (__dpd->TheCurrentCurve.initialPoints[i] + 1) % MOVING_PARAMETRIC_STEPS;
      // shiftCount = (shiftCount + 1) % s_numCoords;
    }
    uint16_t coord = __dpd->TheCurrentCurve.initialPoints[i];
    // uint8_t intensity = max(DEFAULT_MILLIS, _1stBarValue);
    //  int value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
    //   int value = constrain(_1stBarValue, MIN_FFT_DB, MAX_FFT_DB);
    //   value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
    //_TheLeds[_TheMapping.XY(round(s_TheCurrentCurve.xCoord[coord]), round(s_TheCurrentCurve.yCoord[coord]))] = CHSV(HSVHue::HUE_BLUE, 255, (uint8_t)value);

    // if (i > shiftCount && i < ((shiftCount + (s_numCoords / 2)) % s_numCoords)) {
    //     intensity = intensity / 2;
    // }
    // if (i < MOVING_PARAMETRIC_POINTS/2) {
    //     intensity = intensity / 2;
    // }

    _TheLeds[_TheMapping.XY(round(__dpd->TheCurrentCurve.xCoord[coord]), round(__dpd->TheCurrentCurve.yCoord[coord]))] = CHSV(HSVHue::HUE_YELLOW + _1stBarValue / 3, 255, intensity);  //(uint8_t)value);
  }

  // s_frameNum = 0;
  // s_frameNum= (s_frameNum + 1) % 3;

  // i ara pintem el text
  //_u8g2.clearBuffer();
  std::string test;
  if (__dpd->TheDemoParams.currentPhase == 0 && !_Connected2Wifi && !__dpd->alreadyDrawedText) {
    _u8g2.setFont(u8g2_font_oskool_tr);  // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr
    test = "Iniciant FlipaLeds...";
    _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
    __dpd->alreadyDrawedText = true;
  } else if (__dpd->TheDemoParams.currentPhase < (MAX_DEMO_PHASES - 2) && _Connected2Wifi && !__dpd->alreadyDrawedText) {
    _u8g2.clearBuffer();
    //_u8g2.setFont(u8g2_font_profont10_tr); // u8g2_font_tom_thumb_4x6_mn); //u8g2_font_unifont_t_emoticons big emoticons 14pix
    test = "Connectat a la wiFi!";
    _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
    __dpd->alreadyDrawedText = true;
  } else if (__dpd->TheDemoParams.currentPhase >= (MAX_DEMO_PHASES - 2)) {
    if (_Connected2Wifi && !__dpd->alreadyDrawedText) {
      _u8g2.clearBuffer();
      test = Utils::string_format("IP=[%s]", WiFi.localIP().toString().c_str());
      //_u8g2.setFont(u8g2_font_princess_tr); // u8g2_font_tom_thumb_4x6_mn);
      _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
      __dpd->alreadyDrawedText = true;
    } else if (!_Connected2Wifi && newPhase) {
      _u8g2.clearBuffer();
      test = Utils::string_format("Sense WiFi...");
      //_u8g2.setFont(u8g2_font_princess_tr); // u8g2_font_tom_thumb_4x6_mn);
      _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 1) - 1, test.c_str());
    }
  }
  // botmaker_te --> 16x16 funny
  // font_princess_tr --> maca, de princesses
  // emoticons21 --> funny emoticons
  // u8g2_font_tom_thumb_4x6_tn --> es veu bé, però els ":" ocupen un full char
  // u8g2_font_eventhrees_tr --> guay però molt petita, 3x3
  // u8g2_font_micro_tn --> 3x5 molt guay pero ocupa 3 pixels cada char.
  // _u8g2.drawStr(0, (THE_PANEL_HEIGHT / 2)-1, test.c_str());
  //   _u8g2.setFont(u8g2_font_micro_tn); // u8g2_font_tom_thumb_4x6_tn   u8g2_font_blipfest_07_tn);
  //   _u8g2.drawStr(6, 12, theTime.c_str());
  //_ThePanel.SetBaseHue(HSVHue::HUE_YELLOW);
#if defined(PANEL_SIZE_96x48)
  _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 5, 6, __dpd->textPos, THE_PANEL_HEIGHT - 16, __dpd->hue, intensity);
#else
  _ThePanel.DrawScreenBufferXY(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), 2, 3, __dpd->textPos, 16, __dpd->hue, intensity);
#endif

  __dpd->textPos += __dpd->inc;
  if (__dpd->textPos < (-THE_PANEL_WIDTH * 2)) {
    __dpd->inc = 1;
  } else if (__dpd->textPos > THE_PANEL_WIDTH) {
    __dpd->inc = (-1);
  }
  __dpd->hue++;

  if (!_DemoMode && __dpd) {
    delete __dpd;
    __dpd = nullptr;
  }
}

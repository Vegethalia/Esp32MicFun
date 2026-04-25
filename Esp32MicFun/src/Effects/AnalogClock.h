#include <math.h>

class AnalogClock {
 private:
  const int RADIUS;

  struct PointCercle {
    int16_t ledIndex;
    uint8_t hue;
  };

  // Upper bound: ~2 × circumference of ring (r≈25 → ≈314 px) + margin
  static constexpr uint16_t MAX_CIRCLE_POINTS = 400;
  PointCercle _cerclePoints[MAX_CIRCLE_POINTS];
  uint16_t    _numPoints          = 0;
  bool        _cercleInicialitzat = false;

 public:
  // static const int WIDTH = 96;
  // static const int HEIGHT = 54;
  static const int CENTER_X = THE_PANEL_WIDTH / 2;
  static const int CENTER_Y = THE_PANEL_HEIGHT / 2;

  AnalogClock(int radi = 25)
      : RADIUS(radi) {
  }

  void pinta(int hour, int minute, double second) {
    dibuixaCercle();
    dibuixaMarquesHores();

    float secAngle  = (M_PI_2) - (M_TWOPI * second / 60.0f);
    float minAngle  = (M_PI_2) - (M_TWOPI * (minute + second / 60.0f) / 60.0f);
    float hourAngle = (M_PI_2) - (M_TWOPI * ((hour % 12 + minute / 60.0f) / 12.0f));

    dibuixaAgullaGruixuda(hourAngle, static_cast<int>(RADIUS * 0.45), 0.75, HSVHue::HUE_BLUE);
    dibuixaAgullaGruixuda(minAngle,  static_cast<int>(RADIUS * 0.7),  0.45, HSVHue::HUE_AQUA);
    dibuixaAgulla(secAngle, static_cast<int>(RADIUS * 0.8f));

    uint8_t intensity = max((int)100, (int)_1stBarValue);
    _TheLeds[LedIndexXY(CENTER_X, CENTER_Y)] = CHSV(HSVHue::HUE_RED, 255, intensity);
  }

 private:
  void dibuixaAgulla(double angle, int length) {
    for (int i = 1; i <= length; ++i) {
      int x = CENTER_X + static_cast<int>(i * std::cos(angle));
      int y = CENTER_Y - static_cast<int>(i * std::sin(angle));
      if (x >= 0 && x < THE_PANEL_WIDTH && y >= 0 && y < THE_PANEL_HEIGHT) {
        // screen[y][x] = ch;
        _TheLeds[LedIndexXY(x, y)] = CHSV(HSVHue::HUE_RED, 128, 80);  // Color de l'agulla
      }
    }
  }

  void dibuixaAgullaGruixuda(double angle, int length, double thickness, uint8_t hue = HSVHue::HUE_RED) {
    float dx = std::cos(angle);
    float dy = std::sin(angle);

    CHSV hsvColor(hue, 255, 128);

    for (int i = 1; i <= length; ++i) {
      float cx = CENTER_X + i * dx;
      float cy = CENTER_Y - i * dy;

      for (float a = 0; a < 2 * M_PI; a += 1.5f) {
        float ox = std::cos(a) * thickness;
        float oy = std::sin(a) * thickness;

        int x = static_cast<int>(cx + ox);
        int y = static_cast<int>(cy + oy);

        if (x >= 0 && x < THE_PANEL_WIDTH && y >= 0 && y < THE_PANEL_HEIGHT) {
          _TheLeds[LedIndexXY(x, y)] = hsvColor;  // Color de l'agulla
        }
      }
    }
  }

  void dibuixaMarquesHores() {
    uint8_t intensity = max((int)100, (int)_1stBarValue);

    CHSV horaColor(HSVHue::HUE_YELLOW, 255, intensity);

    for (int h = 0; h < 12; ++h) {
      float angle = (M_PI / 2) - (2 * M_PI * h / 12.0);
      int x = CENTER_X + static_cast<int>((RADIUS - 3) * std::cos(angle));
      int y = CENTER_Y - static_cast<int>((RADIUS - 3) * std::sin(angle));
      if (x >= 0 && x < THE_PANEL_WIDTH && y >= 0 && y < THE_PANEL_HEIGHT) {
        _TheLeds[LedIndexXY(x, y)] += horaColor;  // Color de la marca d'hora
      }
    }
  }

  void dibuixaCercle() {
    uint8_t baseHue = (uint8_t)(_TheFrameNumber % 255);

    if (!_cercleInicialitzat) {
      const float step      = 0.0075f;
      const float thickness = 0.5f;
      _numPoints = 0;

      // Dedup bitmap: 1 bit per LED, ~648 bytes heap, freed after init
      constexpr int totalLeds = THE_PANEL_WIDTH * THE_PANEL_HEIGHT;
      uint8_t* visited = (uint8_t*)calloc((totalLeds + 7) / 8, 1);

      uint8_t hue = 0;
      for (float angle = 0; angle < 2 * M_PI; angle += step) {
        for (float r = RADIUS - thickness; r <= RADIUS + thickness; r += 0.3f) {
          int x = CENTER_X + static_cast<int>(r * std::cos(angle));
          int y = CENTER_Y - static_cast<int>(r * std::sin(angle));
          if (x >= 0 && x < THE_PANEL_WIDTH && y >= 0 && y < THE_PANEL_HEIGHT) {
            int16_t idx  = (int16_t)LedIndexXY(x, y);
            uint16_t byte = (uint16_t)idx >> 3;
            uint8_t  bit  = 1u << (idx & 7);
            if (visited && !(visited[byte] & bit)) {
              visited[byte] |= bit;
              if (_numPoints < MAX_CIRCLE_POINTS)
                _cerclePoints[_numPoints++] = { idx, hue };
            }
          }
        }
        hue++;
      }
      if (visited) free(visited);
      _cercleInicialitzat = true;
    }

    for (uint16_t i = 0; i < _numPoints; ++i)
      _TheLeds[_cerclePoints[i].ledIndex].setHSV(_cerclePoints[i].hue + baseHue, 255, 100);
  }
};

void DrawAnalogClock(MsgAudio2Draw& mad) {
  static AnalogClock clock((THE_PANEL_HEIGHT / 2) - 2);

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  clock.pinta(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

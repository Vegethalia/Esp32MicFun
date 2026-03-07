#include <math.h>

#include <unordered_map>
#include <vector>

class AnalogClock {
 private:
  const int RADIUS;

  struct PointCercle {
    int16_t ledIndex;  // Index in the LED array
    uint8_t hue;
    // Constructor to initialize the point
    PointCercle(int16_t index, uint8_t h) : ledIndex(index), hue(h) {}
    PointCercle() : ledIndex(0), hue(0) {}  // Default constructor
  };

  struct pair_hash {
    size_t operator()(const std::pair<int, int>& p) const {
      return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
  };

  std::unordered_map<std::pair<int, int>, PointCercle, pair_hash> _cerclePrecalculat;
  bool _cercleInicialitzat = false;

 public:
  // static const int WIDTH = 96;
  // static const int HEIGHT = 54;
  static const int CENTER_X = THE_PANEL_WIDTH / 2;
  static const int CENTER_Y = THE_PANEL_HEIGHT / 2;

  AnalogClock(int radi = 25)
      : RADIUS(radi) {
  }

  void pinta(int hour, int minute, double second) {
    // std::vector<std::vector<CHSV>> screen(THE_PANEL_HEIGHT, std::vector<CHSV>(THE_PANEL_WIDTH, CHSV(0, 0, 0)));

    dibuixaCercle();
    dibuixaMarquesHores();

    float secAngle = (M_PI_2) - (M_TWOPI * second / 60.0f);
    float minAngle = (M_PI_2) - (M_TWOPI * (minute + second / 60.0f) / 60.0f);
    float hourAngle = (M_PI_2) - (M_TWOPI * ((hour % 12 + minute / 60.0f) / 12.0f));

    dibuixaAgullaGruixuda(hourAngle, static_cast<int>(RADIUS * 0.45), 0.75, HSVHue::HUE_BLUE);
    dibuixaAgullaGruixuda(minAngle, static_cast<int>(RADIUS * 0.7), 0.45, HSVHue::HUE_AQUA);
    dibuixaAgulla(secAngle, static_cast<int>(RADIUS * 0.8f));

    uint8_t intensity = max((int)100, (int)_1stBarValue);
    _TheLeds[LedIndexXY(CENTER_X, CENTER_Y)] = CHSV(HSVHue::HUE_RED, 255, intensity);  // Center dot

    if (_TheFrameNumber % 250 == 0) {
      // SendDebugMessage(Utils::string_format("Analog Clock: %02d:%02d:%02.0f NumPoints=%d", hour, minute, second, _cerclePrecalculat.size()).c_str());
    }
    // screen[CENTER_Y][CENTER_X] = '+';

    // for (const auto& row : screen) {
    //   std::string sLine;
    //   for (char c : row) sLine += c;
    //   sLine += '\n';
    //   std::cout << sLine;
    // }
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
    uint8_t baseHue = _TheFrameNumber % 255;

    if (!_cercleInicialitzat) {
      const float step = 0.0075;
      const float thickness = 0.5;

      for (float angle = 0; angle < 2 * M_PI; angle += step) {
        uint8_t iter = 0;

        for (float r = RADIUS - thickness; r <= RADIUS + thickness; r += 0.3) {
          int x = CENTER_X + static_cast<int>(r * std::cos(angle));
          int y = CENTER_Y - static_cast<int>(r * std::sin(angle));
          if (x >= 0 && x < THE_PANEL_WIDTH && y >= 0 && y < THE_PANEL_HEIGHT) {
            // char pixel = (iter < 1 || iter > 3) ? '.' : chCercle;
            // CHSV hsvPixel = CHSV(baseHue, 255, (iter < 1 || iter > 3) ? 50 : 90);

            // CHSV hsvPixel = CHSV(baseHue, 255, 100);  // Color del cercle
            auto key = std::make_pair(x, y);
            if (_cerclePrecalculat.find(key) == _cerclePrecalculat.end()) {  // || pixel == chCercle) {
              _cerclePrecalculat[key] = PointCercle(LedIndexXY(x, y), baseHue);
            }
          }
          iter++;
        }
        baseHue++;
      }
      _cercleInicialitzat = true;
    }

    // CHSV myPixel;
    for (const auto& element : _cerclePrecalculat) {
      const auto& pos = element.first;  // o el que correspongui
      const auto& pixel = element.second;
      // myPixel.h = baseHue;
      // myPixel.s = 255;                               // o el que correspongui
      // myPixel.v = 100;                               // o el que correspongui
      _TheLeds[pixel.ledIndex].setHSV(pixel.hue + baseHue, 255, 100);  // Center dot
      // baseHue++;
    }
  }
};

void DrawAnalogClock(MsgAudio2Draw& mad) {
  static AnalogClock clock((THE_PANEL_HEIGHT / 2) - 2);

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  clock.pinta(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

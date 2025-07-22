
#if defined(PANEL_SIZE_96x54)
#define CLOCK_HORIZ_PIXELS 55
#define CLOCK_VERT_PIXELS 11  // 7
#elif defined(PANEL_SIZE_64x32)
#define CLOCK_HORIZ_PIXELS 42
#define CLOCK_VERT_PIXELS 9  // 7
#endif
#define CENTER_CLOCK true

void DrawClock(
    uint8_t fontHeight = CLOCK_VERT_PIXELS + 1,
    uint8_t xPos = CENTER_CLOCK ? (THE_PANEL_WIDTH - CLOCK_HORIZ_PIXELS) / 2 : (THE_PANEL_WIDTH - CLOCK_HORIZ_PIXELS)) {
#if defined(PANEL_SIZE_96x54)
  xPos -= 1;  // ens saltem el pixel "txungo" que just cau a la bora d'un número de la hora
  fontHeight += 1;
#endif
  static int baseHue = 0;

  struct tm timeinfo;
  if (_Connected2Wifi) {
    getLocalTime(&timeinfo);
  } else {
    time_t now = time(0);  // secs since boot
    timeinfo = *localtime(&now);
  }
  std::string theTime;
  theTime = Utils::string_format("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  // static int count = 0;
  // if ((count % 25) == 0) {
  //     log_d("Time=%s", theTime.c_str());
  // }
  // count++;

  // botmaker_te --> 16x16 funny
  // font_princess_tr --> maca, de princesses
  // emoticons21 --> funny emoticons
  // u8g2_font_tom_thumb_4x6_tn --> es veu bé, però els ":" ocupen un full char
  // u8g2_font_eventhrees_tr --> guay però molt petita, 3x3
  // u8g2_font_micro_tn --> 3x5 molt guay pero ocupa 3 pixels cada char.
  _u8g2.clearBuffer();
  _u8g2.setFont(u8g2_font_sisterserif_tr);  // bold and big
  //_u8g2.setFont(u8g2_font_cu12_tr);  // u8g2_font_tom_thumb_4x6_mn);
  _u8g2.drawStr(xPos, fontHeight, theTime.c_str());
  //   _u8g2.setFont(u8g2_font_micro_tn); // u8g2_font_tom_thumb_4x6_tn   u8g2_font_blipfest_07_tn);
  //   _u8g2.drawStr(6, 12, theTime.c_str());
  uint8_t intensity;
  if (_NightMode) {
    _ThePanel.SetBaseHue(HSVHue::HUE_BLUE);
    intensity = 164;
  } else {
    _ThePanel.SetBaseHue((uint8_t)(_TheFrameNumber / 4));
    intensity = max((int)164, (int)_1stBarValue);
  }
  _ThePanel.DrawScreenBuffer(_u8g2.getBufferPtr(), _u8g2.getBufferTileWidth(), THE_PANEL_WIDTH, 2, baseHue++, intensity);
  //        FastLED.show();
}

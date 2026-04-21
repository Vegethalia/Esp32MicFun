
// Scrolls the current date and temperatures at the song-name row (same font and position as DrawSongName).
// The clock itself is always drawn separately on top via DrawClock().
void DrawClockWithDataAndTemp(bool resetTextPos = false) {
  static int16_t textPos = THE_PANEL_WIDTH - 2;
  static bool alreadyDrawedText = false;
  static int8_t vert = 0;

  if (resetTextPos) {
    textPos = THE_PANEL_WIDTH - 2;
    alreadyDrawedText = false;
  }

  if (!alreadyDrawedText) {
    time_t now = time(nullptr);
    tm* timeinfo = localtime(&now);

    std::string text = Utils::string_format(
        "%02d/%02d/%04d  golfes:%.1f\xb0  terrassa:%.1f\xb0",
        timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900,
        _LastTempInterior, _LastTempTerrassa);

    _u8g2LongText.clearBuffer();
#if defined(PANEL_SIZE_96x54)
    vert = 2;
    _u8g2LongText.setFont(u8g2_font_t0_12_tf);  // Same font as song name
#elif defined(PANEL_SIZE_64x32)
    _u8g2LongText.setFont(u8g2_font_5x7_tf);
    vert = (-2);
#endif
    _u8g2LongText.drawStr(0, CLOCK_VERT_PIXELS + 1, text.c_str());
    alreadyDrawedText = true;
  }

  uint8_t baseHue = (uint8_t)(_TheFrameNumber / 4);
  uint8_t intensity = std::min(std::max((uint8_t)(75), _1stBarValue), (uint8_t)(200));

  // Multicolor scroll at same position as song name
  bool painting = _ThePanel.DrawScreenBufferXY(_u8g2LongText.getBufferPtr(),
                                               _u8g2LongText.getBufferTileWidth(), 0, 1,
                                               textPos, CLOCK_VERT_PIXELS + vert, baseHue, intensity, true, 128);
  textPos--;

  if (!painting || textPos < (-THE_PANEL_WIDTH * 4)) {
    _DisplayingClockScroll = false;
    log_d("Finished displaying DateAndTemp scroll");
  }
}

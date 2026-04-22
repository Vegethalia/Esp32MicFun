
// Scrolls the current date and temperatures at the bottom of the panel.
// Uses the compact font (u8g2_font_smolfont_tf), same as song names in small mode.
void DrawClockWithDataAndTemp(bool resetTextPos = false, int16_t yOverride = -1) {
  static int16_t textPos = THE_PANEL_WIDTH - 2;
  static bool alreadyDrawedText = false;

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
    _u8g2LongText.setFont(u8g2_font_smolfont_tf);
    _u8g2LongText.drawStr(0, CLOCK_VERT_PIXELS + 1, text.c_str());
    alreadyDrawedText = true;
  }

  uint8_t baseHue = (uint8_t)(_TheFrameNumber / 4);
  uint8_t intensity = std::min(std::max((uint8_t)(75), _1stBarValue), (uint8_t)(200));

  // Multicolor scroll: bottom of panel by default, or song-name row if yOverride is set
  const int16_t drawY = (yOverride >= 0) ? yOverride : (THE_PANEL_HEIGHT - 14);
  bool painting = _ThePanel.DrawScreenBufferXY(_u8g2LongText.getBufferPtr(),
                                               _u8g2LongText.getBufferTileWidth(), 0, 1,
                                               textPos, drawY, baseHue, intensity, true, 128);
  textPos--;

  if (!painting || textPos < (-THE_PANEL_WIDTH * 4)) {
    _DisplayingClockScroll = false;
    log_d("Finished displaying DateAndTemp scroll");
  }
}
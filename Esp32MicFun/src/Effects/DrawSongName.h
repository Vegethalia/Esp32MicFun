
// #include "GlobalDefines.h"

void DrawSongName(const char* pSongName, bool smallFont, bool resetTextPos = false) {
  static int16_t textPos = THE_PANEL_WIDTH - 2;
  static int16_t inc = -1;
  static bool alreadyDrawedText = false;
  static int8_t vert = 0;

  if (resetTextPos) {
    textPos = THE_PANEL_WIDTH - 2;
    inc = -1;
    alreadyDrawedText = false;
  }
  if (!alreadyDrawedText) {
    _u8g2LongText.clearBuffer();
    if (smallFont) {
      _u8g2LongText.setFont(u8g2_font_smolfont_tf);  // u8g2_font_courR08_tf
    } else {
#if defined(PANEL_SIZE_96x54)
      vert = 2;  // 1 pixel down
      _u8g2LongText.setFont(u8g2_font_t0_12_tf);
#elif defined(PANEL_SIZE_64x32)
      _u8g2LongText.setFont(u8g2_font_5x7_tf);  // u8g2_font_6x10_tf
      vert = (-2);
#endif
    }
    _u8g2LongText.drawStr(0, CLOCK_VERT_PIXELS + 1, pSongName);
    //_u8g2.setFont(u8g2_font_princess_tr);  // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr

    alreadyDrawedText = true;
  }
  uint8_t intensity = std::max((uint8_t)(100), _1stBarValue);
  bool painting = _ThePanel.DrawScreenBufferXY(_u8g2LongText.getBufferPtr(), _u8g2LongText.getBufferTileWidth(), 0, 1,
                                               textPos, CLOCK_VERT_PIXELS + vert, HSVHue::HUE_YELLOW, intensity, false, 128);

  // if (_numFrames & 0x01 == 1) { //anem a mitja velocitat
  textPos += inc;
  //}

  if (!painting || (inc < 0 && textPos < (-THE_PANEL_WIDTH * 4)) || (inc > 0 && textPos > THE_PANEL_WIDTH)) {
    if (inc < 0 && textPos < (-THE_PANEL_WIDTH * 4)) {
      inc = 1;  // change direction
    } else if (inc > 0 && textPos > THE_PANEL_WIDTH) {
      inc = -1;  // change direction
      _DisplayingSongName = false;
      log_d("Finished displaying SongName=%s", pSongName);
    }
  }
}

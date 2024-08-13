
void DrawCalculator(MsgAudio2Draw& mad)
{
    static uint8_t __hue = 0;
    static int16_t __xTitlePos = THE_PANEL_WIDTH;

    if (_StartedCalcMode <= 0) {
        _StartedCalcMode = millis();
        __hue = random8();
        _u8g2.setFont(u8g2_font_5x7_tr); // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr
    }
    //    if (millis() - _StartedCalcMode < CALC_MODE_SHOWTITLE_MS*100) {
    //_u8g2.clearBuffer();
    //_u8g2.setFont(u8g2_font_micro_tr); // u8g2_font_tom_thumb_4x6_mn); u8g2_font_princess_tr
    ShowSmallTitle("Super Calculadora", __hue, std::max((uint8_t)(100), _1stBarValue), __xTitlePos--, -1);
    if (_TheFrameNumber % 2 == 0) {
        //__xTitlePos--;
        __hue++;
    }
    if (__xTitlePos < -(THE_PANEL_WIDTH * 2)) {
        __xTitlePos = THE_PANEL_WIDTH;
    }
    //    }
}

void DrawVertSpectrogram(MsgAudio2Draw& mad) {
  static int8_t incValue = +1;
  static int8_t actualInc = 0;
  static const int8_t MAX_INC = 32;

  if (_numFrames % 4 == 0) {
    actualInc += incValue;
    if (actualInc >= MAX_INC) {
      incValue = -1;
    } else if (actualInc == 0) {
      incValue = +1;
    }
  }

  if (_TheDesiredHue < 0) {
    _ThePanel.SetBaseHue((HSVHue::HUE_ORANGE - (MAX_INC / 3)) + actualInc);
  } else {
    _ThePanel.SetBaseHue((_TheDesiredHue - (MAX_INC / 3)) + actualInc);
  }

  assert(THE_PANEL_WIDTH > 1);

  if (_pianoMode) {
    for (uint16_t i = 0; i < _TheMapping.GetWidth(); i++) {
      if (mad.pDBs[i] < 100) {
        mad.pDBs[i] = 0;
      }
    }
  }
  _ThePanel.PushLineWithPrecalcFire(mad.pDBs);
}

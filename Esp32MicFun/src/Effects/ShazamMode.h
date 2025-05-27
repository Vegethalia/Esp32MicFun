
void ProcessShazamMode(MsgAudio2Draw& mad) {
  static uint8_t _baseHue = 0;
  static int8_t _inc = 1;
  static uint8_t _intensity = 1;

  if (_ShazamSongs && !_DemoMode && _TheDrawStyle != DRAW_STYLE::CALC_MODE) {
    auto now = millis();
    bool resetTextPos = false;
    uint32_t timeSinceLastDetection = now - _LastSongDetectionTime;

    if ((now - _LastSongDisplayTime) > 20000) {
      _DisplayingSongName = true;
      _LastSongDisplayTime = now;
      resetTextPos = true;
      // if (timeSinceLastDetection > 65000) {
      //   _DetectedSongName = "Esperant Detecci\xF3...";
      // }
    }

    if (_DisplayingSongName) {
      if (timeSinceLastDetection < 65000) {
        DrawSongName(_DetectedSongName.c_str(), false, resetTextPos);  // update text position....
      } else {
        DrawSongName("Esperant Detecci\xF3...", true, resetTextPos);  // update text position....
      }
    }

    if ((now - _ShazamActivationTime) > SHAZAM_DEACTIVATION_AFTER) {
      _ShazamSongs = false;
      _ThePubSub.publish(TOPIC_DEBUG, "Shazam Mode OFF (porta funcionant 1h!)!!", false);
    }

    // pintem k'icona de "ShazamMode"
    // imaginem que es fa una petició a shazam cada 90..100s.
    // Per tant si pintem 4 pixels, apaguem 1 cada 25s i quan s'apaguen tots, en teoria tindrem refresh de cançó
    int numPixels = 4 - min(timeSinceLastDetection / 26000, (uint32_t)4);
    if (numPixels == 0) {
      numPixels = 1;  // al menys un pixel....
    }
//    if (_numFrames % 5 == 0) {
      _intensity += _inc;

      if (_intensity > 128) {
        _intensity = 128;
        _inc = -1;
      } else if (_intensity < 50) {
        _intensity = 50;
        _inc = 1;
      }
  //  }
    _TheLeds[_TheMapping.XY(THE_PANEL_WIDTH - 1, 0)] = CHSV(HSVHue::HUE_PURPLE, 255, _intensity);
    if (numPixels > 1) {
      _TheLeds[_TheMapping.XY(THE_PANEL_WIDTH - 2, 0)] = CHSV(HSVHue::HUE_PURPLE, 200, _intensity);
    }
    if (numPixels > 2) {
      _TheLeds[_TheMapping.XY(THE_PANEL_WIDTH - 1, 1)] = CHSV(HSVHue::HUE_PURPLE, 150, _intensity);
    }
    if (numPixels > 3) {
      _TheLeds[_TheMapping.XY(THE_PANEL_WIDTH - 2, 1)] = CHSV(HSVHue::HUE_PURPLE, 100, _intensity);
    }
  }
}

void DrawGrays() {
  int j = THE_PANEL_HEIGHT / 2;
  for (int i = THE_PANEL_WIDTH / 3; i < THE_PANEL_WIDTH * 2 / 3; i++) {
    _TheLeds[_TheMapping.XY(i, j + 0)] = CRGB(128, 128, 128);  // Grays
    _TheLeds[_TheMapping.XY(i, j + 1)] = CRGB(128, 128, 128);  // Grays
    _TheLeds[_TheMapping.XY(i, j + 2)] = CRGB(80, 80, 80);     // Grays
    _TheLeds[_TheMapping.XY(i, j + 3)] = CRGB(80, 80, 80);     // Grays
    _TheLeds[_TheMapping.XY(i, j + 4)] = CRGB(64, 64, 64);     // Grays
    _TheLeds[_TheMapping.XY(i, j + 5)] = CRGB(64, 64, 64);     // Grays
    _TheLeds[_TheMapping.XY(i, j + 6)] = CRGB(40, 40, 40);     // Grays
    _TheLeds[_TheMapping.XY(i, j + 7)] = CRGB(40, 40, 40);     // Grays
  }
}

// Task to draw screen.
void vTaskDrawer(void* pvParameters) {
  if (INIT_SCREEN) {
    log_d("In vTaskDrawer. Begin Display...");
    _u8g2.setBusClock(BUS_SPEED);
    log_d("In vTaskDrawer.afterSetBus...");
    _u8g2.begin();
    log_d("In vTaskDrawer.afterBegin...");
    //_u8g2.setFont(u8g2_font_profont12_mf);
    // u8g2_font_tiny_simon_tr --> maca, cuadrada, 3x5 --> 28pixels full time
    //_u8g2.setFont(u8g2_font_nokiafc22_tn); // u8g2_font_squeezed_r6_tn); // u8g2_font_tom_thumb_4x6_mn);
    _u8g2.setFont(u8g2_font_princess_tr);
    log_d("In vTaskDrawer.afterSetFont...");
    _u8g2.setContrast(64);
  }

  log_d("Starting vTaskDrawer...");

  uint8_t lastBuff = 0;
  uint32_t samplesDrawn = 0;
  uint16_t missedFrames = 0;
  MsgAudio2Draw mad;
  WiFiUDP myUdp;
  uint8_t calcDBs[THE_PANEL_WIDTH];
  // WiFiClient myTcp;

  while (true) {
    if (xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY)) {
      //   log_d("vTaskDrawer - Received data from Queue. AudioLen=[%d].", mad.audioLenInSamples);

      int16_t* pDest = mad.pAudio;

      if (_numFrames == 0) {
        _InitTime = millis();
      }

      //_TaskParams.newDataReady = false;
      // if (lastBuff == _TaskParams.lastBuffSet) {
      //     missedFrames++;
      // }
      // lastBuff = _TaskParams.lastBuffSet;
      // log_d("DataReady! DataBuffer=[%d]. BuffNumber=[%d]", _TaskParams.lastBuffSet, _TaskParams.buffNumber);
      if (USE_SCREEN) {
        _u8g2.clearBuffer();
      }

      // int16_t value = 0;

      // if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
      // 	log_d("ShowLeds Queue FULL!!");
      // }
      if (_Drawing) {
        continue;
      }
      _Drawing = true;
      _TheFrameNumber++;

      // Calculate max BASS power among first bars
      /*_1stBarValue = 0;
      for (uint16_t i = 4; i < THE_PANEL_WIDTH / 5; i++) {
          auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
          value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
          if (_1stBarValue < value) {
              _1stBarValue = value;
          }
      }*/
      _1stBarValue = 0;
      mad.pDBs = calcDBs;
      for (uint16_t i = 0; i < THE_PANEL_WIDTH; i++) {
        int8_t value = constrain(mad.pFftMag[i], (int)MIN_FFT_DB, MAX_FFT_DB);
        calcDBs[i] = (uint8_t)map(value, (int)MIN_FFT_DB, MAX_FFT_DB, 0, 255);
        if (i <= 5 && _1stBarValue < calcDBs[i]) {
          _1stBarValue = calcDBs[i];
        }
        // if (_pianoMode && calcDBs[i] < 100) { // 40% of the max value
        //     calcDBs[i] = 0;
        // }
      }
      // continue; //----------------------------------------
      // FastLED.setBrightness(64);

      // if (_ShazamSongs && !_DemoMode && _TheDrawStyle != DRAW_STYLE::CALC_MODE) {
      //   SendAudio(mad, PORT_SHAZAM_SERVICE);  // enviem aqui per que alguna cosa de sota modifica l'audio de mad.audiobuffer
      // }

      if (_DemoMode) {
        FastLED.clear();
        DrawParametric(mad);
        if (_Connected2Wifi) {
          //_DemoMode = false;
        }
      } else {
        switch (_TheDrawStyle) {
          case DRAW_STYLE::BARS_WITH_TOP:
            FastLED.clear();
            DrawWave(mad);
            DrawLedBars(mad);
            DrawClock();
            break;
          case DRAW_STYLE::VERT_FIRE:
            FastLED.clear();
            DrawWave(mad);
            DrawVertSpectrogram(mad, VertSpectrogramStyle::FIRE);  // El pintem primer ja que matxaca lo que ja existeix a _TheLeds per velocitat
            if (_pianoMode) {
              DrawPulses(mad);
            }
            DrawClock();
            break;
          case DRAW_STYLE::HORIZ_FIRE:
            DrawHorizSpectrogram(mad);
            if (!_ShazamSongs) {
              SendAudio(mad, PORT_STREAM_AUDIO);
            }
            break;
          case DRAW_STYLE::VISUAL_CURRENT:
            //_ThePubSub.publish(TOPIC_DEBUG, "Current", false);
            FastLED.clear();
            DrawWave(mad);
            DrawCurrentGraph(mad);
            DrawClock();
            break;
          case DRAW_STYLE::MATRIX_FFT:
            //_ThePubSub.publish(TOPIC_DEBUG, "Current", false);
            FastLED.clear();
            DrawWave(mad);
            DrawMatrixFFT(mad);
            DrawClock();
            break;
          case DRAW_STYLE::DISCO_LIGTHS:
            //_ThePubSub.publish(TOPIC_DEBUG, "Current", false);
            FastLED.clear();
#if defined(PANEL_SIZE_96x54)
            DrawWave(mad);
#endif
            DrawMatrixFFT(mad);
            DrawDiscoLights(mad);
            DrawClock();
            break;
          case DRAW_STYLE::CALC_MODE:
            FastLED.clear();
            // DrawFrame(HUE_PURPLE, std::max((uint8_t)(100), _1stBarValue));
            if (!_AllLinesUsed) {
              DrawMatrixFFT(mad);
            }
            DrawCalculator(mad);
            break;
          case DRAW_STYLE::DRAW_THUMBNAIL:
            FastLED.clear();
            DrawThumbnail();
            break;
        }
      }

      ProcessShazamMode(mad);

      // DrawGrays();

      // DrawClock();
      FastLED.show();
      _Drawing = false;

      _numFrames++;

      auto now = millis();
      if (_numFrames && (now - _InitTime) >= 30000) {
        _fps = 1000.0f / ((float)(now - _InitTime) / (float)_numFrames);
        log_d("time=%d frames=%d fps=%3.2f samplesDrawn=%d missedFrames=%d", now - _InitTime, _numFrames, _fps, samplesDrawn, missedFrames);
        samplesDrawn = 0;
        _numFrames = 0;
        missedFrames = 0;
        _ThePubSub.publish(TOPIC_FPS, Utils::string_format("%3.2f", _fps).c_str(), true);
      }
    }
  }
}

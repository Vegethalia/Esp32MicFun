// Task to draw screen.
void vTaskDrawer(void* pvParameters)
{
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

    uint8_t lastBuff = 0;
    uint32_t samplesDrawn = 0;
    uint16_t missedFrames = 0;
    MsgAudio2Draw mad;
    WiFiUDP myUdp;
    // WiFiClient myTcp;

    while (true) {
        if (xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY)) {
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
            _1stBarValue = 0;
            for (uint16_t i = 4; i < THE_PANEL_WIDTH / 5; i++) {
                auto value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
                value = map(value, MIN_FFT_DB, MAX_FFT_DB, 25, 255);
                if (_1stBarValue < value) {
                    _1stBarValue = value;
                }
            }

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
                    DrawVertSpectrogram(mad); //, VertSpectrogramStyle::RUNING);
                    if (_pianoMode) {
                        DrawPulses(mad);
                    }
                    DrawClock();
                    break;
                case DRAW_STYLE::HORIZ_FIRE:
                    DrawHorizSpectrogram(mad);
                    if (_Connected2Wifi) {
                        // if (!myTcp.connected()) {
                        //     myTcp.connect(IPAddress(192, 168, 1, 141), 4444);
                        // }
                        int err = myUdp.beginPacket(IPAddress(192, 168, 1, 140), 4444);

                        if (WITH_MEMS_MIC) {
                            myUdp.write((uint8_t*)mad.pAudio, mad.audioLenInSamples * sizeof(int16_t));
                        } else {
                            int16_t mapValue;
                            // std::vector<int16_t> maped;
                            // maped.resize(mad.audioLenInSamples);

                            for (int i = 0; i < mad.audioLenInSamples; i++) {
                                mapValue = (int16_t)map(mad.pAudio[i], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, -32000, 32000);
                                // maped[i] = (int16_t)map(mad.pAudio[i], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, -32000, 32000);
                                myUdp.write((uint8_t*)&mapValue, sizeof(int16_t));
                            }
                        }
                        // myTcp.write((uint8_t*)maped.data(), maped.size() * sizeof(int16_t));
                        myUdp.endPacket();
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
                    // DrawWave(mad);
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
                }
            }
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

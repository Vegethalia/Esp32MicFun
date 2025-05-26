
// Task to read samples.
void vTaskReader(void* pvParameters) {
  const uint8_t REAL_BYTES_X_SAMPLE = 4;                                   // BYTES_X_SAMPLE == 3 ? 4 : BYTES_X_SAMPLE;
  uint8_t dataOrig[MIC_BUFFER_SIZE * OVERSAMPLING * REAL_BYTES_X_SAMPLE];  // AUDIO_DATA_OUT * OVERSAMPLING * REAL_BYTES_X_SAMPLE
  size_t buffSize = sizeof(_TaskParams.data1);
  size_t buffSizeOrig = sizeof(dataOrig);
  size_t bytesRead = 0;
  log_d("Estic a la ReaderTask. BuffSize=[%d]. BuffSizeOrig=[%d]", buffSize, buffSizeOrig);
  i2s_event_t adc_i2s_evt;
  uint32_t recInit = millis();
  uint32_t totalSamples = 0;
  uint16_t numCalls = 0;
  uint16_t missedFrames = 0;
  int32_t superMaxMag = -10000;
  uint16_t superMaxBin = 0;
  MsgAudio2Draw mad;
  // uint16_t THE_FFT_SIZE = AUDIO_DATA_OUT * 16;
  FftPower theFFT(FFT_SIZE, AUDIO_DATA_OUT);  // FFT_SIZE, AUDIO_DATA_OUT  * 2
  float* pInputFft = theFFT.GetInputBuffer();
  bool skipNext = false;
  // float hanningPreCalc[REAL_SIZE_TEMP]; // AUDIO_DATA_OUT

  // for (uint16_t i = 0; i < REAL_SIZE_TEMP; i++) {
  //     // apply hann window w[n]=0.5·(1-cos(2Pi·n/N))=sin^2(Pi·n/N)
  //     // auto hann = 0.5f * (1 - cos((2.0f * PI * (float)k) / (float)(AUDIO_DATA_OUT)));
  //     float hann = 0.5f * (1.0f - cos((2.0f * PI * (float)i) / (float)(REAL_SIZE_TEMP - 1)));
  //     hanningPreCalc[i] = hann;
  //     //     *0.25 * sin((2.0f * PI * (float)i) / (float)(AUDIO_DATA_OUT - 1));
  // }

  // Create the FFT config structure
  // fft_config_t* real_fft_plan = fft_init(AUDIO_DATA_OUT, FFT_REAL, FFT_FORWARD, NULL, NULL);
  float freqs_x_bin = (float)(TARGET_SAMPLE_RATE) / (float)FFT_SIZE;
  log_d("Freqs_x_Bin=%3.2f. buffSizeTaskParams.data1=%d dataOrigBuffSize=%d", freqs_x_bin, buffSize, buffSizeOrig);

  memset(_TaskParams.fftMag, -100, sizeof(_TaskParams.fftMag));
  for (;;) {
    if (xQueueReceive(_adc_i2s_event_queue, (void*)&adc_i2s_evt, (portTickType)portMAX_DELAY)) {
      if (adc_i2s_evt.type == I2S_EVENT_RX_DONE) {
        int16_t* pDest = _TaskParams.data1;

        //_TaskParams.lastBuffSet = _TaskParams.lastBuffSet == 2 ? 1 : 2;
        auto err = i2s_read(I2S_NUM_AUDIO, (void*)dataOrig, buffSizeOrig, &bytesRead, portMAX_DELAY);

        if (err != ESP_OK) {
          log_e("is_read error! [%d]", err);
        } else {
          if (bytesRead != buffSizeOrig) {
            log_w("bytesRead=%d expected=%d", (int)bytesRead, (int)buffSizeOrig);
          }

          uint16_t samplesRead = bytesRead / REAL_BYTES_X_SAMPLE;  // BYTES_X_SAMPLE sizeof(uint16_t);
          totalSamples += samplesRead;
          numCalls++;
          if (_OTA.Status() == OtaUpdater::OtaStatus::UPDATING) {
            continue;
          }
          // log_d("-----");
          // for (int iRaw = 0; iRaw < bytesRead - 12; iRaw += 12) {
          //     log_d("%2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X",
          //         dataOrig[iRaw + 0], dataOrig[iRaw + 1], dataOrig[iRaw + 2], dataOrig[iRaw + 3],
          //         dataOrig[iRaw + 4], dataOrig[iRaw + 5], dataOrig[iRaw + 6], dataOrig[iRaw + 7],
          //         dataOrig[iRaw + 8], dataOrig[iRaw + 9], dataOrig[iRaw + 10], dataOrig[iRaw + 11]);

          //     if (iRaw > 100) {
          //         break;
          //     }
          // }
          //_TaskParams.buffNumber++;
          // now we convert the values to mv from 1V to 3V
          // int k=0;
          // log_d("BytesRead=%d buzzSizeOrig=%d SamplesRead=%d TotalSamples=%d", bytesRead, buffSizeOrig, samplesRead, totalSamples);
          // for (int i = 0, k = 0; i < samplesRead; i += 1, k++) {
          int byteIndex = 0;
          int32_t fastValue;
          int32_t overValue;
          for (int i = 0, k = 0; i < samplesRead; i += OVERSAMPLING, k++) {
            byteIndex = i * REAL_BYTES_X_SAMPLE;
            // if (WITH_MEMS_MIC) { // escalem a mateix rang que MAX9814 analog mic
            //     fastValue = ((int32_t*)(dataOrig + byteIndex))[0] >> 12; //>>8 to get a 24 bit value, >>3 to scale down the value to 1/8
            //                                                              // map(dataOrig[i + ov], 0 - MEMS_MIC_MAXVALUE, MEMS_MIC_MAXVALUE,
            //                                                              //  INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
            //                                                              // log_d("Bytes=%2X %2X %2X %2X bitOr=%8X sum=%08d dest=%08d fast=%8X",
            //                                                              //     dataOrig[byteIndex + 3], dataOrig[byteIndex + 2], dataOrig[byteIndex + 1], dataOrig[byteIndex + 0], value24b, value24b,
            //                                                              //     pDest[k], fastValue);
            //     pDest[k] = (int16_t)(fastValue);
            // } else {
            // pDest[k] = 0;
            overValue = 0;
            for (uint8_t ov = 0; ov < OVERSAMPLING; ov++, byteIndex += REAL_BYTES_X_SAMPLE) {
              int32_t fastValue = ((int32_t*)(dataOrig + byteIndex))[0] >> 11;  // 11;  // 12; //>>8 to get a 24 bit value, >>4 to scale down the value to 1/8
              // 2024-10-04 --> >>10 en comptes de >>12 pq el so que obteniem era massa baix. Ara el volum és adecuat (però hi ha més soroll)
              // 2025-05-20 --> >>11 en comptes de >>10 pq el so que obtenim és massa alt (clipping).

              // pDest[k] += (int16_t)(fastValue); // >> 2 to scale down the value to 1 / 4
              overValue += fastValue;
            }
            // }
            pDest[k] = (int16_t)((overValue / OVERSAMPLING));

            // apply hann window w[n]=0.5·(1-cos(2Pi·n/N))=sin^2(Pi·n/N)
            // auto hann = 0.5f * (1 - cos((2.0f * PI * (float)k) / (float)(AUDIO_DATA_OUT)));
            // log_d("%d - %1.4f", i, hann);
            // pInputFft[k] = hanningPreCalc[k] * (float)pDest[k];
            // pInputFft[k] = (float)pDest[k];

            // // i ara escalem el valor. Only needed to paint the wave
            // if (USE_SCREEN) {
            //     pDest[k] = constrain(pDest[k], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
            //     pDest[k] = map(pDest[k], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, 0, SCREEN_HEIGHT - 1);
            // }
          }

          theFFT.AddSamples(pDest, samplesRead / OVERSAMPLING);
          //                    if (!_Drawing) {
          mad.pAudio = pDest;
          mad.audioLenInSamples = samplesRead / OVERSAMPLING;
          mad.pFftMag = _TaskParams.fftMag;
          mad.sizeFftMagVector = sizeof(_TaskParams.fftMag);

          // auto inTime = esp_timer_get_time();
          if (theFFT.Execute(true, 0)) {  // WITH_MEMS_MIC ? 0 : INPUT_0_VALUE
            // auto totalTime = esp_timer_get_time() - inTime;
            // log_d("FFT time=%lldus!!", totalTime);
            //                        if (!_Drawing) {
            // theFFT.GetFreqPower(mad.pFftMag, MAX_FFT_MAGNITUDE, FftPower::BinResolution::AUTO64_3Hz, maxMagI, superMaxMag);
            auto mode = _pianoMode ? FftPower::BinResolution::PIANO64_6Hz : FftPower::BinResolution::AUTO64_6Hz;  // FftPower::BinResolution::AUTO64_3Hz;
#if defined(PANEL_SIZE_96x48)
            if (FFT_SIZE == 4096) {
              mode = _pianoMode ? FftPower::BinResolution::PIANO96_3Hz : FftPower::BinResolution::AUTO96_3Hz;
            }
            else {
              //mode = _pianoMode ? FftPower::BinResolution::PIANO96_6Hz : FftPower::BinResolution::AUTO96_6Hz;
              mode = _pianoMode ? FftPower::BinResolution::PIANO48_6Hz : FftPower::BinResolution::AUTO96_6Hz;
            }
#endif

            if (_TheDrawStyle == DRAW_STYLE::MATRIX_FFT || _TheDrawStyle == DRAW_STYLE::DISCO_LIGTHS) {
              mode = FftPower::BinResolution::MATRIX;
            }
            int32_t maxMag = -1000;
            uint16_t maxBin = 0;
            theFFT.GetFreqPower(mad.pFftMag, mad.sizeFftMagVector, MAX_FFT_MAGNITUDE, mode, maxBin, maxMag);
            mad.max_freq = (uint16_t)(maxBin * freqs_x_bin);
            if (superMaxMag < maxMag) {
              superMaxMag = maxMag;
              superMaxBin = maxBin;
            }

            if (!xQueueSendToBack(_xQueSendAudio2Drawer, &mad, 0)) {
              missedFrames++;
              // log_d("Send2Drawer error!");
            }
            //     } else {
            //         // log_d("MISSED!!");
            //         missedFrames++;
            //     }
          }
          // if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
          // 	log_d("ShowLeds Queue FULL!!");
          // }

          auto now = millis();
          if ((now - recInit) >= 10000) {
            if (superMaxMag < 1000000) {
              MAX_FFT_MAGNITUDE = 1000000;
            } else {
              MAX_FFT_MAGNITUDE = ((superMaxMag / 500000) + 1) * 500000;
              if (MAX_FFT_MAGNITUDE > 12000000) {
                MAX_FFT_MAGNITUDE = 12000000;
              }
            }

            // if (superMaxMag > 2500000) {
            //     MAX_FFT_MAGNITUDE = 3000000;
            // } else if (superMaxMag > 2000000) {
            //     MAX_FFT_MAGNITUDE = 2500000;
            // } else if (superMaxMag > 1500000) {
            //     MAX_FFT_MAGNITUDE = 2000000;
            // } else if (superMaxMag > 1000000) {
            //     MAX_FFT_MAGNITUDE = 1500000;
            // } else if (superMaxMag > 700000) {
            //     MAX_FFT_MAGNITUDE = 1000000;
            // } else {
            //     MAX_FFT_MAGNITUDE = 750000;
            // }

            std::string msg(Utils::string_format("10sec receiving: time=%d totalSamples=%d numCalls=%d maxMag=%d maxFftMag=%dk maxBin=%d maxFreq=%dHz missedFrames=%d",
                                                 now - recInit, totalSamples, numCalls, superMaxMag, (uint32_t)(MAX_FFT_MAGNITUDE / 1000), superMaxBin, (int32_t)(superMaxBin * freqs_x_bin), missedFrames));
            log_d("%s", msg.c_str());
            _ThePubSub.publish(TOPIC_DEBUG, msg.c_str(), false);

            recInit = now;
            totalSamples = 0;
            numCalls = 0;
            superMaxMag = -10000;
            superMaxBin = 0;
            missedFrames = 0;
          }
        }
      }
      // else if(adc_i2s_evt.type == I2S_EVENT_RX_Q_OVF) {
      // 	log_d("RX data dropped!!\n");
      // }
    }
  }
  vTaskDelete(NULL);
}

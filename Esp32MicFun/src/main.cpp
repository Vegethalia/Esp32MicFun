#include <Arduino.h>
#include <FastLED.h>
// #include <NTPClient.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
// #include <WiFi.h>
#include <WiFiClientSecure.h>
#include <memory>
// #include <sstream>
#include <vector>
// #include "wifiFix.h"
#include <HTTPClient.h>

#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include <components/freertos/FreeRTOS-Kernel/include/freertos/queue.h>

#include "mykeys.h"

#include "FftPower.h"
#include "PowerBarsPanel.h"
#include "SharedUtils/OtaUpdater.h"
#include "SharedUtils\Utils.h"

#include "../.pio/libdeps/az-delivery-devkit-v4/FFT/src/fft.h"

#include "Global.h"

#include "Images.h"

#include "Effects.h"

//----------------------
// Advanced declarations
//----------------------
// Tries to reconnect to wifi (if disconnected), or nothing if WiFi is already connected or if last try was before RETRY_WIFI_EVERY_SECS
// Returns true if WiFi is NOW connected and before was not.
// bool Connect2WiFi();
// Connects to the MQTT broquer if not connected.
void Connect2MQTT();
// PubSubClient callback for received messages
void PubSubCallback(char* pTopic, uint8_t* pData, unsigned int dalaLength);
// // Draws an image
// void DrawImage();
// // Draw a parametric curve
// void DrawParametric();
// Processes the received http payload in csv form and populates the global VisualCurrentConsumption structures
void ProcessCurrentPayload(std::string& thePayload);

/// @brief Configures the NTP server. In this project the timezone and offsets are hardcoded to suit my needs
void ConfigureNTP()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    //    if (timeinfo.tm_year < (2020 - 1900)) {
    //        log_i("Time is not set yet. Connecting to WiFi and getting time over NTP.");
    if (WiFi.isConnected()) { // if its not connected, the ntp server might crash (bug, probably solved already)
        configTime(3600, 3600, "pool.ntp.org");
        sntp_set_sync_interval(60000);
        sntp_restart();
    }
    // update 'now' variable with current time
    time(&now);
    // } else {
    //     sntp_set_sync_interval(60000);
    //     sntp_restart();
    //     setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Configure the TZ. We already have the time set. Andorra time!
    //     tzset();
    // }
}

// bool Connect2WiFi()
// {
//     if (WiFi.isConnected()) {
//         return false; // false because was already connected
//     }
//     auto temps = millis() / 1000;

//     if ((temps >= RETRY_WIFI_EVERY_SECS / 2 && temps < RETRY_WIFI_EVERY_SECS * 2) || (temps - _LastCheck4Wifi) >= RETRY_WIFI_EVERY_SECS * 5) {
//         _LastCheck4Wifi = temps;
//         log_d("[%d] Trying WiFi connection to [%s]", millis(), WIFI_SSID);
//         auto err = WiFi.begin(WIFI_SSID, WIFI_PASS); // FROM mykeys.h
//         err = (wl_status_t)WiFi.waitForConnectResult();
//         if (err != wl_status_t::WL_CONNECTED) {
//             log_d("WiFi connection FAILED! Error=[%d]. Will retry later", err);
//             return false;
//         } else {
//             log_d("WiFi CONNECTED!");
//             // _TheNTPClient.begin();
//             // _TheNTPClient.setTimeOffset(7200); // 3600
//             ConfigureNTP();
//             return true;
//         }
//     }
//     return false; // Too soon to retry, wait.
// }

// Task to read samples.
// FftPower theFFT(FFT_SIZE, AUDIO_DATA_OUT);
// uint16_t dataOrig[AUDIO_DATA_OUT * OVERSAMPLING];
void vTaskReader(void* pvParameters)
{
    uint16_t dataOrig[AUDIO_DATA_OUT * OVERSAMPLING];
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
    MsgAudio2Draw mad;
    // uint16_t THE_FFT_SIZE = AUDIO_DATA_OUT * 16;
    FftPower theFFT(FFT_SIZE, AUDIO_DATA_OUT); // AUDIO_DATA_OUT  * 2
    float* pInputFft = theFFT.GetInputBuffer();
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
    log_d("Freqs_x_Bin=%3.2f", freqs_x_bin);

    memset(_TaskParams.fftMag, -100, sizeof(_TaskParams.fftMag));
    for (;;) {
        if (xQueueReceive(_adc_i2s_event_queue, (void*)&adc_i2s_evt, (portTickType)portMAX_DELAY)) {
            if (adc_i2s_evt.type == I2S_EVENT_RX_DONE) {
                uint16_t* pDest = _TaskParams.data1;

                //_TaskParams.lastBuffSet = _TaskParams.lastBuffSet == 2 ? 1 : 2;
                auto err = i2s_read(I2S_NUM_0, (void*)dataOrig, buffSizeOrig, &bytesRead, portMAX_DELAY);
                if (err != ESP_OK) {
                    log_e("is_read error! [%d]", err);
                } else {
                    if (bytesRead != buffSizeOrig) {
                        log_w("bytesRead=%d expected=%d", (int)bytesRead, (int)buffSizeOrig);
                    }

                    uint16_t samplesRead = bytesRead / sizeof(uint16_t);
                    totalSamples += samplesRead;
                    numCalls++;
                    if (_OTA.Status() == OtaUpdater::OtaStatus::UPDATING) {
                        continue;
                    }
                    //_TaskParams.buffNumber++;
                    // now we convert the values to mv from 1V to 3V
                    // int k=0;
                    // log_d("BytesRead=%d buzzSizeOrig=%d SamplesRead=%d TotalSamples=%d", bytesRead, buffSizeOrig, samplesRead, totalSamples);
                    // for (int i = 0, k = 0; i < samplesRead; i += 1, k++) {
                    for (int i = 0, k = 0; i < samplesRead; i += OVERSAMPLING, k++) {
                        pDest[k] = 0;
                        for (uint8_t ov = 0; ov < OVERSAMPLING; ov++) {
                            pDest[k] += (uint16_t)(esp_adc_cal_raw_to_voltage(dataOrig[i + ov] & MASK_12BIT, _adc_chars));
                        }
                        pDest[k] = pDest[k] / OVERSAMPLING;

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
                    uint16_t maxMagI = 0;
                    mad.pAudio = pDest;
                    mad.audioLenInSamples = samplesRead / OVERSAMPLING;
                    mad.pFftMag = _TaskParams.fftMag;
                    // auto inTime = esp_timer_get_time();
                    if (theFFT.Execute(false, INPUT_0_VALUE)) {
                        // auto totalTime = esp_timer_get_time() - inTime;
                        // log_d("FFT time=%lldus!!", totalTime);
                        if (!_Drawing) {
                            // theFFT.GetFreqPower(mad.pFftMag, MAX_FFT_MAGNITUDE, FftPower::BinResolution::AUTO64_3Hz, maxMagI, superMaxMag);
                            theFFT.GetFreqPower(mad.pFftMag, MAX_FFT_MAGNITUDE,
                                _pianoMode ? FftPower::BinResolution::PIANO64_6Hz : FftPower::BinResolution::AUTO64_3Hz,
                                maxMagI, superMaxMag);
                            mad.max_freq = (uint16_t)(maxMagI * freqs_x_bin);

                            if (!xQueueSendToBack(_xQueSendAudio2Drawer, &mad, 0)) {
                                missedFrames++;
                            }
                        } else {
                            log_d("MISSED!!");
                            missedFrames++;
                        }
                    }
                    //                  } else {
                    //                     log_d("MISSED!!");
                    //                     missedFrames++;
                    //                }
                    // DrawLedBars(mad);
                    //  DrawLeds(mad);

                    // if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
                    // 	log_d("ShowLeds Queue FULL!!");
                    // }
                    auto now = millis();
                    if ((now - recInit) >= 30000) {
                        // std::string msg(Utils::string_format("1sec receiving: time=%d totalSamples=%d numCalls=%d maxMag=%d missedFrames=%d",
                        //     now - recInit, totalSamples, numCalls, superMaxMag, missedFrames));
                        // log_d(msg.c_str());
                        // _ThePubSub.publish(TOPIC_DEBUG, msg.c_str(), false);

                        recInit = now;
                        totalSamples = 0;
                        numCalls = 0;
                        superMaxMag = -10000;
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

// // Task to draw the LEDS
// void vTaskDrawLeds(void* pvParameters)
// {
//     log_d("In vTaskDrawLeds...");
//     MsgAudio2Draw mad;

//     uint32_t lastShow = 0;

//     while (true) {
//         if (xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY) && (millis() - lastShow) > 100) {
//             uint32_t init = millis();
//             DrawLedBars(mad);
//             lastShow = millis();
//             // log_d("ShowTime=%dms", millis()-init);
//         }
//     }

//     vTaskDelete(NULL);
// }

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
            uint16_t* pDest = mad.pAudio;

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
            _Drawing = true;
            _TheFrameNumber++;

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
                    DrawVertSpectrogram(mad);
                    DrawClock();
                    break;
                case DRAW_STYLE::HORIZ_FIRE:
                    DrawHorizSpectrogram(mad);
                    {
                        std::vector<int16_t> maped;
                        maped.resize(mad.audioLenInSamples);
                        //
                        if (_Connected2Wifi) {
                            // if (!myTcp.connected()) {
                            //     myTcp.connect(IPAddress(192, 168, 1, 141), 4444);
                            // }
                            int16_t mapValue;
                            int err = myUdp.beginPacket(IPAddress(192, 168, 1, 140), 4444);
                            for (int i = 0; i < mad.audioLenInSamples; i++) {
                                mapValue = (int16_t)map(mad.pAudio[i], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, -32000, 32000);
                                // maped[i] = (int16_t)map(mad.pAudio[i], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, -32000, 32000);
                                myUdp.write((uint8_t*)&mapValue, sizeof(int16_t));
                            }
                            // myTcp.write((uint8_t*)maped.data(), maped.size() * sizeof(int16_t));
                            myUdp.endPacket();
                        }
                        // _ThePubSub.publish(TOPIC_LIVEAUDIO, reinterpret_cast<uint8_t*>(maped.data()), mad.audioLenInSamples * sizeof(uint16_t));
                        // uint16_t ff = 0xffff;
                        // _ThePubSub.publish(TOPIC_LIVEAUDIO, reinterpret_cast<uint8_t*>(&ff), sizeof(uint16_t));
                    }
                    break;
                case DRAW_STYLE::VISUAL_CURRENT:
                    //_ThePubSub.publish(TOPIC_DEBUG, "Current", false);
                    FastLED.clear();
                    DrawWave(mad);
                    DrawCurrentGraph(mad);
                    DrawClock();
                    break;
                }
            }
            // DrawClock();
            FastLED.show();
            _Drawing = false;

            _numFrames++;

            auto now = millis();
            if (_numFrames && (now - _InitTime) >= 30000) {
                _fps = 1000.0f / ((now - _InitTime) / _numFrames);
                log_d("time=%d frames=%d fps=%3.2f samplesDrawn=%d missedFrames=%d", now - _InitTime, _numFrames, _fps, samplesDrawn, missedFrames);
                samplesDrawn = 0;
                _numFrames = 0;
                missedFrames = 0;
                _ThePubSub.publish(TOPIC_FPS, Utils::string_format("%3.2f", _fps).c_str(), true);
            }
        }
    }
}

void vTaskShowLeds(void* pvParameters)
{
    uint8_t state = 0;
    uint16_t frames = 0;
    uint32_t startMillis = 0;

    while (true) {
        if (frames == 0) {
            startMillis = millis();
        }
        if (_OTA.Status() != OtaUpdater::OtaStatus::UPDATING && !_Drawing) {
            //        s_state++;
            if (state == 1) {
                FastLED.show(_MAX_MILLIS);
                state = 0;
            } else {
                FastLED.show(0);
                state = 1;
            }
            frames++;
        } else if (_OTA.Status() == OtaUpdater::OtaStatus::UPDATING) {
            sleep(250);
        }
        if (frames == 100) {
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("RefreshLeds [%d] Total Time[%d]", frames, (uint32_t)(millis() - startMillis)).c_str(), true);
            frames = 0;
        }
    }
}

void vTaskWifiReconnect(void* pvParameters)
{
    bool reconnected = false;

    auto lastCheck = millis();
    // WiFiClientFixed httpWifiClient;
    // HTTPClient theHttpClient;
    WiFiClientSecure* __httpWifiClient = new WiFiClientSecure();
    HTTPClient* __theHttpClient = new HTTPClient();
    std::string theUrl;
    time_t now;

    sleep(20); // wait a bit...

    _OTA.Setup("MicFun", "", 3434);
    WiFi.mode(WIFI_STA);

    while (true) {
        _Connected2Wifi = false;
        reconnected = false;
        if (WiFi.isConnected()) {
            _Connected2Wifi = true;
        } else {
            auto temps = millis() / 1000;

            if ((temps - _LastCheck4Wifi) >= RETRY_WIFI_EVERY_SECS) {
                _LastCheck4Wifi = temps;
                log_i("[%d] Trying WiFi connection to [%s]", millis(), WIFI_SSID);
                auto err = WiFi.begin(WIFI_SSID, WIFI_PASS); // FROM mykeys.h
                err = (wl_status_t)WiFi.waitForConnectResult();
                if (err != wl_status_t::WL_CONNECTED) {
                    log_e("WiFi connection FAILED! Error=[%d]. Will retry later", err);
                } else {
                    log_i("WiFi CONNECTED!");
                    // _TheNTPClient.begin();
                    // _TheNTPClient.setTimeOffset(7200);
                    _Connected2Wifi = true;
                    reconnected = true;
                    ConfigureNTP();
                }
            }
        }
        if (reconnected) {
            _OTA.Begin();
        }
        if (_Connected2Wifi) {
            _ThePubSub.loop(); // allow the pubsubclient to process incoming messages
            _OTA.Process();
            // _TheNTPClient.update();
            if (!_ThePubSub.connected()) {
                _ThePubSub.setBufferSize(1024);
                Connect2MQTT();
                if (_ThePubSub.connected()) {
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("MicFun connected with IP=[%s]", WiFi.localIP().toString().c_str()).c_str(), true);
                }
            }
        }

        if ((_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) && _Connected2Wifi && millis() > 120000 && (((millis() - lastCheck) > (max(_AgrupaConsumsPerMinuts / 2, 1) * 60000)) || _UpdateCurrentNow)) {
            //_ThePubSub.publish(TOPIC_DEBUG, "Trying SSL connection...", false);
            _UpdateCurrentNow = false;
            lastCheck = millis();
            now = time(nullptr);
            uint8_t mapMax = GetMapMaxPixels();
            // construïm la url ?dataFi=1685583107&numValors=60&maxKWh=4,5&mapejarDe0a=20&agruparPer=2&code=
            theUrl = Utils::string_format("%sdataFi=%lld&numValors=%d&maxWh=%d&mapejarDe0a=%d&agruparPer=%d&csvOutput=1&code=%s",
                CONSUM_ELECTRICITAT_URL, (int64_t)now, (THE_PANEL_WIDTH - 1), _MaxWhToShow, mapMax, _AgrupaConsumsPerMinuts, CONSUM_ELECTRICITAT_KEY);
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("ConsumURL=[%s]", theUrl.c_str()).c_str(), false);

            __httpWifiClient->setInsecure();

            __theHttpClient->setConnectTimeout(2000);
            //__theHttpClient->setReuse(false);
            if (__theHttpClient->begin(*__httpWifiClient, theUrl.c_str())) {
                __theHttpClient->setConnectTimeout(2000);
                __theHttpClient->addHeader("Accept", "*/*");
                int httpResponse = __theHttpClient->GET();
                if (httpResponse > 0) {
                    std::string thePayload = __theHttpClient->getString().c_str();
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("HTTP response [%d] length=[%d]", httpResponse, thePayload.length()).c_str(), false);
                    if (httpResponse == HTTP_CODE_OK) {
                        // log_i("Payload=[%s]", thePayload.c_str());
                        ProcessCurrentPayload(thePayload);
                    }
                } else {
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("ConsumURL=[%s]", theUrl.c_str()).c_str(), false);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("GET returned code [%d]", httpResponse).c_str(), false);
                }
            } else {
                _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Begin returned false :(").c_str(), false);
            }
            __theHttpClient->end();
        }
        delay(2000);
    }
}

// void vTaskRefrescarConsumElectricitat(void* pvParameters)
// {
//     auto lastCheck = millis();
//     // WiFiClientFixed httpWifiClient;
//     // HTTPClient theHttpClient;
//     WiFiClientSecure* __httpWifiClient = new WiFiClientSecure();
//     HTTPClient* __theHttpClient = new HTTPClient();
//     std::string theUrl;
//     time_t now;

//     while (true) {
//         if (!_Connected2Wifi || (millis() - lastCheck) < UPDATE_CONSUM_ELECTRICITAT_CADA_MS) {
//             delay(10000); // sense preses...
//             continue;
//         }
//         lastCheck = millis();
//         now = time(nullptr);
//         uint8_t mapMax = 20;
//         if (_MapMaxWhToPixels == 1000 || _MapMaxWhToPixels == 2000 || _MapMaxWhToPixels == 2500 || _MapMaxWhToPixels == 4000 || _MapMaxWhToPixels == 5000) {
//             mapMax = 20; // 20/10/8/5/4 leds per kWh
//         } else if (_MapMaxWhToPixels == 1500 || _MapMaxWhToPixels == 3000 || _MapMaxWhToPixels == 3500) {
//             mapMax = 21; // 14/7/6 leds per kWh
//         } else if (_MapMaxWhToPixels == 4500 || _MapMaxWhToPixels == 6000) {
//             mapMax = 18; // 4/3 leds per kWh
//         } else if (_MapMaxWhToPixels == 5500) {
//             mapMax = 22; // 4 leds per kWh
//         }
//         // construïm la url ?dataFi=1685583107&numValors=60&maxKWh=4,5&mapejarDe0a=20&agruparPer=2&code=
//         theUrl = Utils::string_format("%sdataFi=%lld&numValors=%d&maxWh=%d&mapejarDe0a=%d&agruparPer=%d&code=%s",
//             CONSUM_ELECTRICITAT_URL, (int64_t)now, THE_PANEL_WIDTH, _MaxWhToShow, mapMax, _AgrupaConsumsPerMinuts, CONSUM_ELECTRICITAT_KEY);
//         _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("ConsumURL=[%s]", theUrl.c_str()).c_str(), false);

//         __httpWifiClient->setInsecure();

//         log_d("Free Heap=%d largest_block=%d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

//         __theHttpClient->setConnectTimeout(2000);
//         __theHttpClient->setReuse(false);
//         if (__theHttpClient->begin(*__httpWifiClient, theUrl.c_str())) {
//             __theHttpClient->setTimeout(20000);
//             // theHttpClient.addHeader("host", "visualcurrentapp.azurewebsites.net");
//             // theHttpClient.addHeader("Content-Type", "application/json");
//             __theHttpClient->addHeader("Accept", "*/*");
//             // theHttpClient.addHeader("Connection", "keep-alive");
//             int httpResponse = __theHttpClient->GET();
//             if (httpResponse > 0) {
//                 _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("HTTP GET ok! Code=[%d]", httpResponse).c_str(), false);
//                 std::string thePayload = __theHttpClient->getString().c_str();
//                 _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("HTTP response length=[%d]", thePayload.length()).c_str(), false);
//                 log_i("Payload=[%s]", thePayload.c_str());
//             } else {
//                 _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("GET returned code [%d]", httpResponse).c_str(), false);
//             }
//         } else {
//             _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Begin returned false :(").c_str(), false);
//         }
//         __theHttpClient->end();
//     }
// }

void Connect2MQTT()
{
    if (!_ThePubSub.connected()) {
        _ThePubSub.setClient(_TheWifi);
        _ThePubSub.setServer(MQTT_BROKER, MQTT_PORT);
        _ThePubSub.setCallback(PubSubCallback);
        //		String s = WiFi.macAddress());
        if (!_ThePubSub.connect((String("ESP32_Espectrometer") + WiFi.macAddress()[0]).c_str())) {
            log_e("ERROR!! PubSubClient was not able to connect to PiRuter!!");
        } else { // Subscribe to the feeds
            log_i("PubSubClient connected to PiRuter MQTT broker!!");
            _ThePubSub.publish(TOPIC_DEBUG, "PubSubClient connected to PiRuter MQTT broker!!", true);

            if (!_ThePubSub.subscribe(TOPIC_INTENSITY)) {
                log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_INTENSITY);
            }
            if (!_ThePubSub.subscribe(TOPIC_DELAYFRAME)) {
                log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_DELAYFRAME);
            }
            if (!_ThePubSub.subscribe(TOPIC_STYLE)) {
                log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_STYLE);
            }
            if (!_ThePubSub.subscribe(TOPIC_RESET)) {
                log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_RESET);
            }
            if (!_ThePubSub.subscribe(TOPIC_NIGHTMODE)) {
                log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_NIGHTMODE);
            }
            if (!_ThePubSub.subscribe(TOPIC_GROUPMINUTS)) {
                log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_GROUPMINUTS);
            }
            // if (!_ThePubSub.subscribe(TOPIC_FPS)) {
            //     log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_FPS);
            // }
        }
    }
}

void setup()
{
    Serial.begin(115200);
    // wait for serial monitor to open
    while (!Serial)
        ;

    // pinMode(PIN_BASS_LED, OUTPUT);
    pinMode(PIN_DATA_LEDS1, OUTPUT);
    pinMode(PIN_DATA_LEDS2, OUTPUT);
    pinMode(PIN_DATA_LEDS3, OUTPUT);
    pinMode(PIN_DATA_LEDS4, OUTPUT);
    pinMode(PIN_AUDIO_IN, INPUT);

    FastLED.addLeds<WS2812B, PIN_DATA_LEDS1, GRB>(_TheLeds, 0, THE_PANEL_WIDTH * 8);
    FastLED.addLeds<WS2812B, PIN_DATA_LEDS2, GRB>(_TheLeds, THE_PANEL_WIDTH * 8, THE_PANEL_WIDTH * 8);
    FastLED.addLeds<WS2812B, PIN_DATA_LEDS3, GRB>(_TheLeds, THE_PANEL_WIDTH * 8 * 2, THE_PANEL_WIDTH * 8);
    FastLED.addLeds<WS2812B, PIN_DATA_LEDS4, GRB>(_TheLeds, THE_PANEL_WIDTH * 8 * 3, THE_PANEL_WIDTH * 8);
    // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);
    FastLED.setTemperature(Halogen);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 20000);
    FastLED.setBrightness(_MAX_MILLIS);

    _ThePanel.SetParams(&_TheLeds, &_TheMapping);

    //_TheLeds.fill_rainbow(HSVHue::HUE_YELLOW);

    // range 0...4096
    adc1_config_width(ADC_WIDTH_BIT_12);
    // full voltage range
    adc1_config_channel_atten(PIN_AUDIO_IN, ADC_ATTEN_DB_11);
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, _adc_chars);
    // Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        log_d("calibration=eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        log_d("calibration=Two Point");
    } else {
        log_d("calibration=Default");
    }

    // esp_err_t status = adc_vref_to_gpio(ADC_UNIT_1, ADC1_CHANNEL_4_GPIO_NUM);
    // if(status == ESP_OK) {
    // 	printf("v_ref routed to GPIO\n");
    // }
    // else {
    // 	printf("failed to route v_ref\n");
    // }
    log_d("Installing Driver...");
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = SAMPLE_RATE, // samplerate configured are the number of samples returned if channel=Left+Right, not frequency. So we need to multiply*2.
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB, // I2S_COMM_FORMAT_STAND_MSB, I2S_COMM_FORMAT_I2S_MSB
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1, default 0
        .dma_buf_count = 2, // 8,
        .dma_buf_len = (AUDIO_DATA_OUT * OVERSAMPLING), // AUDIO_DATA_OUT*OVERSAMPLING, //these are the number of samples we will read from i2s_read
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0 //,//SAMPLE_RATE,
                        // .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
                        // .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
    };
    // install and start i2s driver
    auto err = i2s_driver_install(I2S_NUM_0, &i2s_config, _adc_i2s_event_queue_size, &_adc_i2s_event_queue);
    if (err != ESP_OK) {
        log_e("driver install failed");
    }
    // i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    static const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE, // Sample f(Hz) (= sample f * 2)  on this pin (optional).
        .ws_io_num = I2S_PIN_NO_CHANGE, // Left/Right   (= sample f)      on this pin (optional).
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_AUDIO_IN // RTCIO_CHANNEL_4_GPIO_NUM // ADC1_CHANNEL_4_GPIO_NUM// RTCIO_CHANNEL_4_GPIO_NUM
    };
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        log_e("i2s_set_pin failed");
    }
    // init ADC pad
    err = i2s_set_adc_mode(ADC_UNIT_1, PIN_AUDIO_IN); // ADC1_CHANNEL_4
    if (err != ESP_OK) {
        log_e("set_adc_mode failed");
    }
    // //enable de ADC
    err = i2s_adc_enable(I2S_NUM_0);

    // Create queue to communicate reader & drawer
    _xQueSendAudio2Drawer = xQueueCreate(1, sizeof(MsgAudio2Draw));
    _xQueSendFft2Led = xQueueCreate(1, sizeof(MsgAudio2Draw));
    // start task to read samples from I2S
    // xTaskCreate(vTaskReader, "ReaderTask", 5000, (void*)&_TaskParams, 4, &_readerTaskHandle); // 7000
    xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 5000, (void*)&_TaskParams, 4, &_readerTaskHandle, 0); // 7000
    // // xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 2048, (void*)&_TaskParams, 2, &_readerTaskHandle, 0);
    // start task to draw screen
    xTaskCreatePinnedToCore(vTaskDrawer, "Draw Leds", 2500, (void*)&_TaskParams, 3, &_drawTaskHandle, 1);
    //    xTaskCreatePinnedToCore(vTaskShowLeds, "vTaskShowLeds", 2048, (void*)&_TaskParams, 2, &_drawTaskShowLeds, 1);
    // // start task to draw leds
    // // xTaskCreatePinnedToCore(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle, 0);
    // //	xTaskCreate(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle);

    // xTaskCreate(vTaskWifiReconnect, "Wifi Reconnect", 4000, nullptr, 4, &_wifiReconnectTaskHandle); // 4096
    xTaskCreatePinnedToCore(vTaskWifiReconnect, "Wifi Reconnect", 4500, nullptr, 4, &_wifiReconnectTaskHandle, 1); // 4096

    // xTaskCreate(vTaskRefrescarConsumElectricitat, "Refrescar Consum", 20000, nullptr, 2, &_refrescarConsumTaskHandle);
    //** xTaskCreatePinnedToCore(vTaskRefrescarConsumElectricitat, "Refrescar Consum", 15000, nullptr, 2, &_refrescarConsumTaskHandle, 1);
}

// PLAY WITH MATRIX
//  #define TEMP_LEDS (16*32*2) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32
//  CRGBArray<TEMP_LEDS> _MyTempLeds;
//  uint16_t _indexPix = 0;
//  bool added = false;
//  CLEDController* _MyTempController = nullptr;
// END PLAY WITH MATRIX

void loop()
{
    // PLAY WITH MATRIX
    // if(!added) {
    //     _MyTempController = &FastLED.addLeds<WS2812B, PIN_DATA_LEDS, GRB>(_MyTempLeds, TEMP_LEDS);
    //     _MyTempController->setTemperature(Halogen);
    //     FastLED.setBrightness(20);
    // }
    // _MyTempLeds.fadeToBlackBy(10);
    // _MyTempLeds[_indexPix] = CHSV(HSVHue::HUE_PURPLE, random8(), 100);
    // _indexPix++;
    // _indexPix %= TEMP_LEDS;
    // _MyTempController->showLeds();
    // END PLAY WITH MATRIX

    // fill_rainbow(_TheLeds, NUM_LEDS, initial, thisSpeed);
    // initial+=thisSpeed;
    // FastLED.show();

    // static int pos = 0;
    // static int temp = 0;
    // for (int i = 0; i < 33; i++) {
    //     for (int j = 0; j < 16; j++) {
    //         _TheLeds[_TheMapping.XY((i + pos) % 33, j)] = __imgArletErola2[i][j];
    //         napplyGamma_video(_TheLeds[_TheMapping.XY((i + pos) % 33, j)], 1.8f);
    //     }
    // }
    // pos++;

    // FastLED.show();
    //  delay(_delayFrame);

    //  DrawParametric();
    // DrawSparks();
    delay(10000);
    log_d("Main: Free Heap=%d largest_block=%d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    if (_Connected2Wifi) {
        _ThePubSub.publish(TOPIC_FREEHEAP, Utils::string_format("%d", esp_get_free_heap_size()).c_str());
        _ThePubSub.publish(TOPIC_BIGGESTFREEBLOCK, Utils::string_format("%d", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)).c_str());
        _ThePubSub.publish(TOPIC_HIWATER_READER, Utils::string_format("%d", uxTaskGetStackHighWaterMark(_readerTaskHandle)).c_str());
        _ThePubSub.publish(TOPIC_HIWATER_WIFI, Utils::string_format("%d", uxTaskGetStackHighWaterMark(_wifiReconnectTaskHandle)).c_str());
        _ThePubSub.publish(TOPIC_HIWATER_DRAWER, Utils::string_format("%d", uxTaskGetStackHighWaterMark(_drawTaskHandle)).c_str());
    }
}

void PubSubCallback(char* pTopic, uint8_t* pData, unsigned int dataLenght)
{
    std::string theTopic(pTopic);
    std::string theMsg;

    for (uint16_t i = 0; i < dataLenght; i++) {
        theMsg.push_back((char)pData[i]);
    }
    log_v("Received message from [%s]: [%s]", theTopic.c_str(), theMsg.c_str());

    if (theTopic.find(TOPIC_INTENSITY) != std::string::npos) {
        auto origIntensity = _MAX_MILLIS;
        auto newIntensity = min(std::atoi(theMsg.c_str()), 255); //(int)MAX_MILLIS);
        _MAX_MILLIS = newIntensity;

        if (newIntensity != origIntensity) {
            // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);
            FastLED.setBrightness(_MAX_MILLIS);

            //     // log_d("Changing led intensity=%d", newIntensity);
            //     _Intensity = newIntensity;
            //     float percentPower = (float)_Intensity / (float)MAX_INTENSITY;
            //     _IntensityMAmps = (uint32_t)(((MAX_TARGET_CURRENT - MIN_TARGET_CURRENT) * percentPower) + MIN_TARGET_CURRENT);
            //     FastLED.setMaxPowerInVoltsAndMilliamps(5, _IntensityMAmps); // FastLED power management set at 5V, 1500mA
            //     _ThePubSub.publish(TOPIC_INTENSITY, Utils::string_format("%d", _Intensity).c_str(), true);
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated intensity=%dmAhs", _MAX_MILLIS).c_str(), true);
        }
    }
    if (theTopic.find(TOPIC_DELAYFRAME) != std::string::npos) {
        _delayFrame = max(std::atoi(theMsg.c_str()), (int)0);
        _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated delay=%dms", _delayFrame).c_str(), true);
    }
    if (theTopic.find(TOPIC_STYLE) != std::string::npos) {
        _TheDrawStyle = (DRAW_STYLE)max(min(std::atoi(theMsg.c_str()), (int)DRAW_STYLE::MAX_STYLE), 1);
        if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
            _UpdateCurrentNow = true;
        }
        _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), true);
    }
    if (theTopic.find(TOPIC_RESET) != std::string::npos) {
        bool resetNow = std::atoi(theMsg.c_str()) != 0;
        if (resetNow) {
            ESP.restart();
        }
    }
    if (theTopic.find(TOPIC_NIGHTMODE) != std::string::npos) {
        byte nightOn = std::atoi(theMsg.c_str()) != 0;
        if (nightOn) {
            _MAX_MILLIS = NIGHT_MILLIS;
            FastLED.setBrightness(_MAX_MILLIS);
            _NightMode = true;
            _TheDrawStyle = DRAW_STYLE::BARS_WITH_TOP;
        } else if (_NightMode) {
            FastLED.setBrightness(DEFAULT_MILLIS);
            _TheDrawStyle = DRAW_STYLE::VERT_FIRE;
            _NightMode = false;
        }
    }
    if (theTopic.find(TOPIC_GROUPMINUTS) != std::string::npos) {
        int minuts = std::atoi(theMsg.c_str());
        _AgrupaConsumsPerMinuts = max(min(minuts, (int)60), 1);
        _UpdateCurrentNow = true;
        _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Electricitat agrupada per [%d] minuts", (int)_AgrupaConsumsPerMinuts).c_str(), true);
    }
    // _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Received Topic=[%s] Msg=[%s]", theTopic.c_str(), theMsg.c_str()).c_str(), true);
}

void ProcessCurrentPayload(std::string& theCsvPayload)
{
    std::string line;
    //, swh;
    // uint32_t epochTime, wh;

    if (!_pLectures) {
        _pLectures = new LecturaConsum[THE_PANEL_WIDTH];
    }
    memset(_pLectures, 0, sizeof(LecturaConsum) * THE_PANEL_WIDTH);
    uint8_t nIndex = 0, nLastNonZero = 0;

    size_t startPos = 0;
    size_t endPos = theCsvPayload.find('\n');

    while (endPos != std::string::npos) {
        line = theCsvPayload.substr(startPos, endPos - startPos);
        //_ThePubSub.publish(TOPIC_DEBUG, line.c_str());

        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            _pLectures[nIndex].horaConsum = atoi(line.substr(0, commaPos).c_str());
            _pLectures[nIndex].valorEnLeds = atoi(line.substr(commaPos + 1).c_str());
            // _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("hora=%d valor=%d", _pLectures[nIndex].horaConsum, _pLectures[nIndex].valorEnLeds).c_str());
            if (_pLectures[nIndex].horaConsum) {
                nLastNonZero = nIndex;
            }
            nIndex++;
        }

        startPos = endPos + 1;
        endPos = theCsvPayload.find('\n', startPos);
    }

    // Processar l'última línia (si n'hi ha)
    if (startPos < theCsvPayload.length()) {
        line = theCsvPayload.substr(startPos);

        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            _pLectures[nIndex].horaConsum = atoi(line.substr(0, commaPos).c_str());
            _pLectures[nIndex].valorEnLeds = atoi(line.substr(commaPos + 1).c_str());
            if (_pLectures[nIndex].horaConsum) {
                nLastNonZero = nIndex;
            }
            nIndex++;
        }
    }
    if (nIndex > 0 && _lastCurrentTime < _pLectures[nLastNonZero].horaConsum) {
        _ThePubSub.publish(TOPIC_CURRENT_WH, Utils::string_format("%d", _pLectures[nLastNonZero].valorEnLeds).c_str());
        _lastCurrentTime = _pLectures[nLastNonZero - 1].horaConsum;
    } else {
        _ThePubSub.publish(TOPIC_DEBUG, "NO DATA");
    }
}

#include <Arduino.h>
#include <FastLED.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <memory>
#include <vector>

#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include <components/freertos/FreeRTOS-Kernel/include/freertos/queue.h>

#include "mykeys.h"

#include "FftPower.h"
#include "PowerBarsPanel.h"
#include "SharedUtils/OtaUpdater.h"
#include "SharedUtils\Utils.h"

#include "Global.h"

#include "Images.h"

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
//Draws an image
void DrawImage();

bool Connect2WiFi()
{
    if (WiFi.isConnected()) {
        return false; // false because was already connected
    }
    auto temps = millis() / 1000;

    if (temps < 3 || (temps - _LastCheck4Wifi) >= RETRY_WIFI_EVERY_SECS) {
        _LastCheck4Wifi = temps;
        log_d("[%d] Trying WiFi connection to [%s]", millis(), WIFI_SSID);
        auto err = WiFi.begin(WIFI_SSID, WIFI_PASS); // FROM mykeys.h
        err = (wl_status_t)WiFi.waitForConnectResult();
        if (err != wl_status_t::WL_CONNECTED) {
            log_d("WiFi connection FAILED! Error=[%d]. Will retry later", err);
            return false;
        } else {
            log_d("WiFi CONNECTED!");
            _TheNTPClient.begin();
            _TheNTPClient.setTimeOffset(3600);
            return true;
        }
    }
    return false; // Too soon to retry, wait.
}

void DrawLedBars(MsgAudio2Draw& mad)
{
    // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);

    static uint32_t lastSuperBass = 0;
    static uint32_t lastLowBass = 0;
    static uint32_t lastTouch = 0;
    static constexpr int lowBass = ((MIN_FFT_DB + MAX_FFT_DB) / 2);
    static constexpr int hiBass = (MIN_FFT_DB / 6);
    static constexpr int touchMinMs = 200;
    static constexpr int touchMaxMs = 500;
    static constexpr int minTimeBetweenTouches = 1000;
    static uint8_t state = 0; // 0=waiting for low. 1=with hi bass. 2=with low after hi

    uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
    uint8_t maxBassValue = 0;
    int16_t value = 0;

    if (VISUALIZATION == FftPower::AUTO34) {
        maxIndex = 34;
    }
    //_TheLeds.fill_solid(CRGB(1,1,1));
    FastLED.clear();
    assert(BAR_HEIGHT > 1);
    uint8_t minBoostBin = (uint8_t)(maxIndex * 0.33); // the first 13 bars in 33 width panel
    constexpr float maxTrebleBoost = 30.0;
    constexpr float minBassBoost = 1.0;
    float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)maxIndex);

/*    bool isHiBass = false, isLowBass = false;
    auto now = millis();
    if (mad.pFftMag[1] >= hiBass) { // || mad.pFftMag[2] >= hiBass) {
        isHiBass = true;
        lastSuperBass = now;
    } else if (mad.pFftMag[1] < lowBass) { //|| mad.pFftMag[2] < lowBass) {
        isLowBass = true;
        lastLowBass = now;
    }
    if (state == 0 && isHiBass && (now - lastLowBass) > touchMinMs && (now - lastLowBass) < touchMaxMs) {
        state = 1;
    } else if (state == 1 && isLowBass && (now - lastTouch) > minTimeBetweenTouches) {
        //_ThePanel.IncBaseHue(41);
        lastTouch = now;
        state = 0;
    } else {
        state = 0;
    }
*/
    // // lets find out the max power bins
    // struct orderedBinByPow {
    //     uint16_t binNum;
    //     int32_t fftMag;
    //     uint8_t brightness;
    // };
    // std::vector<orderedBinByPow> binByPow;
    // for (uint16_t i = 0; i < maxIndex; i++) {
    //     binByPow.push_back({ i, mad.pFftMag[i], 32 });
    // }
    // binByPow[0].fftMag = MIN_FFT_DB;
    // std::sort(binByPow.begin(), binByPow.end(),
    //     [](orderedBinByPow& a, orderedBinByPow& b) {
    //         return a.fftMag < b.fftMag;
    //     });
    // for (uint16_t i = 0; i < maxIndex; i++) {
    //     if (i < 4) {
    //         binByPow[i].brightness = 200;
    //     } else if (i < 8) {
    //         binByPow[i].brightness = 160;
    //     } else if (i < 12) {
    //         binByPow[i].brightness = 120;
    //     } else if (i < 16) {
    //         binByPow[i].brightness = 100;
    //     } else if (i < 20) {
    //         binByPow[i].brightness = 80;
    //     } else {
    //         binByPow[i].brightness = 64;
    //     }
    // }

    //DrawImage();
    for (uint16_t i = 1; i < maxIndex; i++) {
        if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
            auto boost = 1.0f + (i * freqBoost);
            value = (int)(value * boost);
        }

        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, (BAR_HEIGHT * 10) + 9); // fins a 89

        _ThePanel.DrawBar(i - 1, value, 200);
    }
    /*    for (uint16_t i = 0; i < maxIndex; i++) {
            uint8_t bin = binByPow[i].binNum;
            if(bin==0) {
                continue;
            }
            // if (i > minBoostBin) { // boost hi frequencies (to make them more visible)
            //     auto boost = 1.0f + (bin * freqBoost);
            //     value = (int)(value * boost);
            // }

            value = constrain(binByPow[i].fftMag, MIN_FFT_DB, MAX_FFT_DB);
            value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, (BAR_HEIGHT * 10) + 9); // fins a 89

            _ThePanel.DrawBar(bin - 1, value, binByPow[i].brightness);
        }*/

    FastLED.show();
}

void DrawLedsSingleLine(MsgAudio2Draw& mad)
{
    uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
    uint8_t maxBassValue = 0;
    int16_t value = 0;

    if (VISUALIZATION == FftPower::AUTO34) {
        maxIndex = 34;
    }
    // log_d("maxIndex=%d", maxIndex);
    for (uint16_t i = 1; i < maxIndex; i++) {
        value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
        value = map(value, MIN_FFT_DB, MAX_FFT_DB, 1, 128);

        if (value > 100) {
            _TheLeds[NUM_LEDS - i] = CHSV(HSVHue::HUE_AQUA, 255, 128);
        }
        // else if(value<=2) {
        // 	_TheLeds[NUM_LEDS - i - 1] = CRGB(1, 1, 1);//CHSV(HSVHue::HUE_PURPLE, 255, value);
        // }
        else {
            _TheLeds[NUM_LEDS - i] = CHSV(HSVHue::HUE_BLUE, 255, value); // CRGB(value, value, value);//CHSV(HSVHue::HUE_PURPLE, 255, value);
        }
        // if(i<4 && value>maxBassValue) { //BASS bins
        // 	maxBassValue = value;
        // }
    }
    FastLED.show();
}

// Task to read samples.
void vTaskReader(void* pvParameters)
{
    size_t buffSize = sizeof(_TaskParams.data1);
    size_t buffSizeOrig = sizeof(_TaskParams.dataOrig);
    size_t bytesRead = 0;
    log_d("Estic a la ReaderTask. BuffSize=[%d]", buffSize);
    i2s_event_t adc_i2s_evt;
    uint32_t recInit = millis();
    uint32_t totalSamples = 0;
    uint16_t numCalls = 0;
    uint16_t missedFrames = 0;
    int32_t superMaxMag = -10000;
    MsgAudio2Draw mad;
    FftPower theFFT(AUDIO_DATA_OUT);
    float* pInputFft = theFFT.GetInputBuffer();

    // Create the FFT config structure
    // fft_config_t* real_fft_plan = fft_init(AUDIO_DATA_OUT, FFT_REAL, FFT_FORWARD, NULL, NULL);
    float freqs_x_bin = (float)(TARGET_SAMPLE_RATE) / (float)AUDIO_DATA_OUT;
    log_d("Freqs_x_Bin=%3.2f", freqs_x_bin);

    for (;;) {
        if (xQueueReceive(_adc_i2s_event_queue, (void*)&adc_i2s_evt, (portTickType)portMAX_DELAY)) {
            if (adc_i2s_evt.type == I2S_EVENT_RX_DONE) {
                uint16_t* pDest = _TaskParams.lastBuffSet == 2 ? _TaskParams.data1 : _TaskParams.data2;

                _TaskParams.lastBuffSet = _TaskParams.lastBuffSet == 2 ? 1 : 2;
                auto err = i2s_read(I2S_NUM_0, (void*)_TaskParams.dataOrig, buffSizeOrig, &bytesRead, portMAX_DELAY);
                if (err != ESP_OK) {
                    log_e("is_read error! [%d]", err);
                } else {
                    if (bytesRead != buffSizeOrig) {
                        log_w("bytesRead=%d expected=%d", (int)bytesRead, (int)buffSizeOrig);
                    }
                    uint16_t samplesRead = bytesRead / sizeof(uint16_t);
                    totalSamples += samplesRead;
                    numCalls++;
                    //_TaskParams.buffNumber++;
                    // now we convert the values to mv from 1V to 3V
                    // int k=0;
                    // log_d("BytesRead=%d buzzSizeOrig=%d SamplesRead=%d TotalSamples=%d", bytesRead, buffSizeOrig, samplesRead, totalSamples);
                    for (int i = 0, k = 0; i < samplesRead; i += OVERSAMPLING, k++) {
                        pDest[k] = 0;
                        for (uint8_t ov = 0; ov < OVERSAMPLING; ov++) {
                            pDest[k] += (uint16_t)(esp_adc_cal_raw_to_voltage(_TaskParams.dataOrig[i + ov] & MASK_12BIT, _adc_chars));
                        }
                        pDest[k] = pDest[k] / OVERSAMPLING;

                        // apply hann window w[n]=0.5·(1-cos(2Pi·n/N))=sin^2(Pi·n/N)
                        auto hann = 0.5f * (1 - cos((2.0f * PI * (float)k) / (float)AUDIO_DATA_OUT));
                        // log_d("%d - %1.4f", i, hann);
                        pInputFft[k] = hann * (float)pDest[k];
                        //pInputFft[k] = (float)pDest[k];

                        // i ara escalem el valor
                        if (USE_SCREEN) {
                            pDest[k] = constrain(pDest[k], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
                            pDest[k] = map(pDest[k], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, 0, SCREEN_HEIGHT - 1);
                        }
                    }
                    uint16_t maxMagI = 0;
                    mad.pAudio = pDest;
                    mad.pFftMag = _TaskParams.fftMag;
                    theFFT.Execute();
                    theFFT.GetFreqPower(mad.pFftMag, MAX_FFT_MAGNITUDE,
                        BARS_RESOLUTION == 2 ? FftPower::ALL : BARS_RESOLUTION == 4 ? FftPower::HALF
                                                                                    : FftPower::AUTO34,
                        maxMagI, superMaxMag);
                    mad.max_freq = (uint16_t)(maxMagI * freqs_x_bin);

                    if (!xQueueSendToBack(_xQueSendAudio2Drawer, &mad, 0)) {
                        missedFrames++;
                    }
                    // DrawLedBars(mad);
                    //  DrawLeds(mad);

                    // if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
                    // 	log_d("ShowLeds Queue FULL!!");
                    // }
                    auto now = millis();
                    if ((now - recInit) >= 10000) {
                        log_d("1sec receiving: time=%d totalSamples=%d numCalls=%d maxMag=%d missedFrames=%d", now - recInit, totalSamples, numCalls, superMaxMag, missedFrames);
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

// Task to draw the LEDS
void vTaskDrawLeds(void* pvParameters)
{
    log_d("In vTaskDrawLeds...");
    MsgAudio2Draw mad;

    uint32_t lastShow = 0;

    while (true) {
        if (xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY) && (millis() - lastShow) > 100) {
            uint32_t init = millis();
            DrawLedBars(mad);
            lastShow = millis();
            // log_d("ShowTime=%dms", millis()-init);
        }
    }

    vTaskDelete(NULL);
}

// Task to draw screen.
void vTaskDrawer(void* pvParameters)
{
    if (USE_SCREEN) {
        log_d("In vTaskDrawer. Begin Display...");
        u8g2.setBusClock(BUS_SPEED);
        log_d("In vTaskDrawer.afterSetBus...");
        u8g2.begin();
        log_d("In vTaskDrawer.afterBegin...");
        u8g2.setFont(u8g2_font_profont12_mf);
        log_d("In vTaskDrawer.afterSetFont...");
        u8g2.setContrast(64);
    }

    uint8_t lastBuff = 0;
    uint32_t samplesDrawn = 0;
    uint16_t missedFrames = 0;
    MsgAudio2Draw mad;

    while (true) {
        if (xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY)) {
            uint16_t* pDest = mad.pAudio;

            if (_numFrames == 0) {
                _InitTime = millis();
            }

            //_TaskParams.newDataReady = false;
            if (lastBuff == _TaskParams.lastBuffSet) {
                missedFrames++;
            }
            lastBuff = _TaskParams.lastBuffSet;
            // log_d("DataReady! DataBuffer=[%d]. BuffNumber=[%d]", _TaskParams.lastBuffSet, _TaskParams.buffNumber);
            u8g2.clearBuffer();

            // Now the FFT
            int16_t value = 0;

            // u8g2.setDrawColor(1);
            // uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
            // uint8_t barW = BARS_RESOLUTION == 2 ? 1 : BARS_RESOLUTION == 4 ? 2 : 4;
            // uint8_t adjust = BARS_RESOLUTION == 2?0:1;
            // uint8_t maxBassValue=0;
            // for(uint16_t i = 1; i < maxIndex; i++) {
            // 	value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
            // 	value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, SCREEN_HEIGHT - 1);
            // 	// u8g2.drawLine(i, SCREEN_HEIGHT - value, i, SCREEN_HEIGHT-1); --> ALL
            // 	// u8g2.drawLine(i*2, SCREEN_HEIGHT - value, i*2, SCREEN_HEIGHT-1); --> HALF
            // 	u8g2.drawBox(i * barW, SCREEN_HEIGHT - value, barW - adjust, value); //--> QUARTER/AUTO
            // 	if(i<4 && value>maxBassValue) { //BASS bins
            // 		maxBassValue=value;
            // 	}
            // }
            // if(maxBassValue > (SCREEN_HEIGHT - (SCREEN_HEIGHT/4))) {
            // 	//u8g2.setContrast(255);
            // 	_BassOn=true;
            // 	// digitalWrite(PIN_BASS_LED, HIGH);
            // }
            // else {
            // 	_BassOn=false;
            // 	// digitalWrite(PIN_BASS_LED, LOW);
            // 	//u8g2.setContrast(64);
            // 	//u8g2.setContrast(map(maxBassValue, 0, SCREEN_HEIGHT - 1, 1, 128));
            // }
            // //now we simulate that the bars are made of "boxes"
            // u8g2.setDrawColor(0);
            // for(uint16_t i = 4; i < SCREEN_HEIGHT; i += 6) {
            // 	u8g2.drawLine(0, i, SCREEN_WIDTH - 1, i);
            // 	u8g2.drawLine(0, i + 1, SCREEN_WIDTH - 1, i + 1);
            // 	//u8g2.drawLine(0, i + 2, SCREEN_WIDTH - 1, i + 2);
            // }

            // Now The Wave
            if (USE_SCREEN) {
                u8g2.setDrawColor(2);

                // busquem el pass per "0" després de la muntanya més gran
                uint16_t pas0 = 0;
                uint16_t maxAmp = (SCREEN_HEIGHT / 2);
                uint16_t iMaxAmp = 0;
                for (int i = 0; i < (AUDIO_DATA_OUT - SCREEN_WIDTH - SCREEN_WIDTH / 3); i++) {
                    if (pDest[i] < maxAmp) {
                        maxAmp = value;
                        iMaxAmp = i;
                        break;
                    }
                } // ja tenim l'index del pic de la muntanya mes gran. Ara busquem a on creuem per 0
                for (int i = iMaxAmp; i < (AUDIO_DATA_OUT - SCREEN_WIDTH); i++) {
                    if (pDest[i] >= (SCREEN_HEIGHT / 2)) {
                        pas0 = i;
                        break;
                    }
                }
                for (uint16_t i = pas0; i < (pas0 + SCREEN_WIDTH); i++) {
                    value = constrain(pDest[i], INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE);
                    value = map(value, INPUT_0_VALUE - VOLTATGE_DRAW_RANGE, INPUT_0_VALUE + VOLTATGE_DRAW_RANGE, 0, SCREEN_HEIGHT - 1);
                    u8g2.drawPixel(i - pas0, pDest[i]);
                }
                samplesDrawn += AUDIO_DATA_OUT;

                // And finally the text
                u8g2.setFontMode(1);
                u8g2.setDrawColor(2);
                u8g2.drawStr(5, 15, Utils::string_format("FPS=%3.2f", _fps).c_str());

                u8g2.sendBuffer();
            }
            _numFrames++;

            // if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
            // 	log_d("ShowLeds Queue FULL!!");
            // }
            DrawLedBars(mad);

            auto now = millis();
            if (_numFrames && (now - _InitTime) >= 10000) {
                _fps = 1000.0f / ((now - _InitTime) / _numFrames);
                log_d("time=%d frames=%d fps=%3.2f samplesDrawn=%d missedFrames=%d", now - _InitTime, _numFrames, _fps, samplesDrawn, missedFrames);
                samplesDrawn = 0;
                _numFrames = 0;
                missedFrames = 0;
            }
        }
    }
}

void vTaskWifiReconnect(void* pvParameters)
{
    bool reconnected = false;

    _OTA.Setup();
    WiFi.mode(WIFI_STA);

    while (true) {
        _Connected2Wifi = false;
        reconnected = false;
        if (WiFi.isConnected()) {
            _Connected2Wifi = true;
        } else {
            auto temps = millis() / 1000;

            if (temps < 3 || (temps - _LastCheck4Wifi) >= RETRY_WIFI_EVERY_SECS) {
                _LastCheck4Wifi = temps;
                log_i("[%d] Trying WiFi connection to [%s]", millis(), WIFI_SSID);
                auto err = WiFi.begin(WIFI_SSID, WIFI_PASS); // FROM mykeys.h
                err = (wl_status_t)WiFi.waitForConnectResult();
                if (err != wl_status_t::WL_CONNECTED) {
                    log_e("WiFi connection FAILED! Error=[%d]. Will retry later", err);
                } else {
                    log_i("WiFi CONNECTED!");
                    _TheNTPClient.begin();
                    _TheNTPClient.setTimeOffset(3600);
                    _Connected2Wifi = true;
                    reconnected = true;
                }
            }
        }
        if (reconnected) {
            _OTA.Begin();
        }
        if (_Connected2Wifi) {
            _ThePubSub.loop(); // allow the pubsubclient to process incoming messages
            _OTA.Process();
            _TheNTPClient.update();
            if (!_ThePubSub.connected()) {
                Connect2MQTT();
            }
        }
        delay(2000);
    }
}

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
        }
    }
}

void setup()
{
    Serial.begin(115200);
    // wait for serial monitor to open
    while (!Serial)
        ;

    pinMode(PIN_BASS_LED, OUTPUT);
    pinMode(PIN_DATA_LEDS, OUTPUT);

    FastLED.addLeds<WS2812B, PIN_DATA_LEDS, GRB>(_TheLeds, NUM_LEDS);
    // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);
    FastLED.setTemperature(Halogen);
    _ThePanel.SetParams(&_TheLeds, &_TheMapping);

    //_TheLeds.fill_rainbow(HSVHue::HUE_YELLOW);

    // range 0...4096
    adc1_config_width(ADC_WIDTH_BIT_12);
    // full voltage range
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
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
        .communication_format = I2S_COMM_FORMAT_I2S_MSB, // I2S_COMM_FORMAT_STAND_MSB,
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
        .data_in_num = RTCIO_CHANNEL_4_GPIO_NUM // ADC1_CHANNEL_4_GPIO_NUM// RTCIO_CHANNEL_4_GPIO_NUM
    };
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        log_e("i2s_set_pin failed");
    }
    // init ADC pad
    err = i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_4);
    if (err != ESP_OK) {
        log_e("set_adc_mode failed");
    }
    // //enable de ADC
    err = i2s_adc_enable(I2S_NUM_0);

    // Create queue to communicate reader & drawer
    _xQueSendAudio2Drawer = xQueueCreate(1, sizeof(MsgAudio2Draw));
    _xQueSendFft2Led = xQueueCreate(1, sizeof(MsgAudio2Draw));
    // start task to read samples from I2S
    xTaskCreate(vTaskReader, "ReaderTask", 4096, (void*)&_TaskParams, 3, &_readerTaskHandle);
    // // xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 2048, (void*)&_TaskParams, 2, &_readerTaskHandle, 0);
    // start task to draw screen
  //  xTaskCreate(vTaskDrawer, "Draw Screen", 2048, (void*)&_TaskParams, 3, &_drawTaskHandle);
    // // xTaskCreatePinnedToCore(vTaskDrawer, "Draw Screen", 2048, (void*)&_TaskParams, 2, &_drawTaskHandle, 1);
    // // start task to draw leds
    // // xTaskCreatePinnedToCore(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle, 0);
    // //	xTaskCreate(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle);

    xTaskCreate(vTaskWifiReconnect, "Wifi Reconnect", 3072, nullptr, 3, &_wifiReconnectTaskHandle);
}

// uint8_t thisSpeed = 3;
// uint8_t initial = 1;
int _delayFrame = 100;

void DrawParametric()
{
    static uint16_t currPos = 0;
    static uint16_t sizePoses = 8; // sizeof(__2to3PiHalf.initialPoints) / sizeof(uint16_t);
    static HSVHue theHues[8] = { HSVHue::HUE_AQUA, HSVHue::HUE_BLUE, HSVHue::HUE_GREEN, HSVHue::HUE_ORANGE, HSVHue::HUE_PINK, HSVHue::HUE_PURPLE, HSVHue::HUE_RED, HSVHue::HUE_YELLOW };

    FastLED.clear();
    //_TheLeds.fadeLightBy(90);

    // for (int i = 0; i < sizePoses; i++) {
    //     _TheLeds[_TheMapping.XY(
    //         round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
    //         round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
    //         = CHSV(theHues[i], 255, 200);
    // }
    for (int i = 0; i < sizePoses; i++) {
        float x = __2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
        float y = __2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
        int x1 = (int)x + 1;
        int y1 = (int)y + 1;
        float x1Percent = x - (int)x;
        float x0Percent = 1.0 - x1Percent;
        float y1Percent = y - (int)y;
        float y0Percent = 1.0 - x1Percent;
        float c0Percent = (x0Percent + y0Percent) / 2;
        float c1Percent = (x1Percent + y1Percent) / 2;

        if (x1 >= _TheMapping.GetWidth() || y1 >= _TheMapping.GetHeight()) {
            _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255);
        } else {
            _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255 * c0Percent);
            _TheLeds[_TheMapping.XY((int)x1, (int)y1)] = CHSV(theHues[i], 255, 255 * c1Percent);
        }

        // _TheLeds[_TheMapping.XY(round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]), round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
        //     = CHSV(theHues[i], 255, 200);
    }

    ++currPos;
    // currPos = currPos % 252;
    FastLED.show();
    if (_delayFrame) {
        delay(_delayFrame);
    }
}

void DrawImage()
{
    static int pos = 0;
    static int temp = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 16; j++) {
            _TheLeds[_TheMapping.XY((i + pos) % 33, j)] = __imgArletErola2[i][j];
            napplyGamma_video(_TheLeds[_TheMapping.XY((i + pos) % 33, j)], 1.8f);
        }
    }
    pos++;
    // FastLED.show();
}

void loop()
{
    // if(_BassOn) {
    // 	digitalWrite(PIN_BASS_LED, HIGH);
    // }
    // else {
    // 	digitalWrite(PIN_BASS_LED, LOW);
    // }

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
    //delay(_delayFrame);

        DrawParametric();
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
}

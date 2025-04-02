#include <Arduino.h>

// #define FASTLED_ESP32_I2S
// #define FASTLED_OVERCLOCK 1.10
#include <FastLED.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include <WiFiClientSecure.h>

#include <memory>
#include <vector>

#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include <components/freertos/FreeRTOS-Kernel/include/freertos/queue.h>

#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

#include "mykeys.h"
// #include "Images.h"

#include "FftPower.h"
#include "PowerBarsPanel.h"
#include "SharedUtils/OtaUpdater.h"
#include "SharedUtils\Utils.h"
// Global variables and functions. need the previous includes to work properly
#include "Global.h"

//----------------------
// Advanced declarations
//----------------------
/// @brief Sets default parameters for night mode on/off
/// @param nightOn
void SetNightMode(bool nightOn);
/// @brief Persists a given preference value.
void UpdatePref(Prefs thePref);
/// @brief Processes a change in the drawing style, restoring the required parameters.
void ProcessStyleChange(DRAW_STYLE oldStyle, DRAW_STYLE newStyle);
/// @brief Changes the drawing style. Calls ProcessStyleChange.
bool ChangeDrawStyle(DRAW_STYLE style, bool forceChange = false);

#include "Effects.h"  //uses some of the vars declared in Global
#include "Tasks.h"    //tasks might call some of the functions declared above

void setup() {
  Serial.begin(115200);
  // wait for serial monitor to open
  while (!Serial);

  // pinMode(PIN_BASS_LED, OUTPUT);
  pinMode(PIN_DATA_LEDS1, OUTPUT);
  pinMode(PIN_DATA_LEDS2, OUTPUT);
  pinMode(PIN_DATA_LEDS3, OUTPUT);
  pinMode(PIN_DATA_LEDS4, OUTPUT);
  pinMode(PIN_AUDIO_IN, INPUT);
  pinMode(PIN_MIC_I2S_SCK, OUTPUT);
  pinMode(PIN_MIC_I2S_WS, OUTPUT);
  pinMode(PIN_MIC_I2S_SD, INPUT);

  FastLED.addLeds<WS2812B, PIN_DATA_LEDS1, GRB>(_TheLeds, 0, THE_PANEL_WIDTH * 8);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS2, GRB>(_TheLeds, THE_PANEL_WIDTH * 8, THE_PANEL_WIDTH * 8);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS3, GRB>(_TheLeds, THE_PANEL_WIDTH * 8 * 2, THE_PANEL_WIDTH * 8);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS4, GRB>(_TheLeds, THE_PANEL_WIDTH * 8 * 3, THE_PANEL_WIDTH * 8);

  _ThePanel.SetParams(&_TheLeds, &_TheMapping);

  _ThePrefs.begin("FlipaLeds", false);  // false=read+write
  _TheDrawStyle = (DRAW_STYLE)(_ThePrefs.getUChar(PREF_STYLE, DRAW_STYLE::DEFAULT_STYLE));
  _MAX_MILLIS = _ThePrefs.getUShort(PREF_INTENSITY, _MAX_MILLIS);
  _AgrupaConsumsPerMinuts = _ThePrefs.getUShort(PREF_CURRENT_BY_MINUTES, DEFAULT_CONSUM_PER_MINUTS);
  //_NightMode = _ThePrefs.getBool(PREF_NIGHTMODE, _NightMode); //night mode is always off at start
  _pianoMode = _ThePrefs.getBool(PREF_PIANOMODE, _pianoMode);
  _TheDesiredHue = _ThePrefs.getInt(PREF_CUSTOM_HUE, -1);
  _DaylightSaving = _ThePrefs.getBool(PREF_DAYLIGHT_SAVING, true);  // Horari d'estiu. uses _DaylightSaving.

  //_TheDrawStyle = DRAW_STYLE::VERT_FIRE;

  log_d(Utils::string_format("Loaded defaults: Style=%d Intensity=%d ConsumsPerMinut=%d NightMode=%d PianoMode=%d Hue=%d",
                             (int)_TheDrawStyle, (int)_MAX_MILLIS, (int)_AgrupaConsumsPerMinuts, (int)_NightMode, (int)_pianoMode, (int)_TheDesiredHue)
            .c_str());

  // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);
  FastLED.setTemperature(Halogen);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 20000);
  FastLED.setBrightness(_MAX_MILLIS);
  FastLED.setDither(0);

  if (_NightMode) {
    SetNightMode(true);
  }

  //_TheLeds.fill_rainbow(HSVHue::HUE_YELLOW);

  if (!WITH_MEMS_MIC) {
    // range 0...4096
    adc1_config_width(ADC_WIDTH_BIT_12);
    // full voltage range
    adc1_config_channel_atten(PIN_AUDIO_IN, ADC_ATTEN_DB_12);
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, DEFAULT_VREF, _adc_chars);
    // Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      log_d("calibration=eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
      log_d("calibration=Two Point");
    } else {
      log_d("calibration=Default");
    }
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
      .mode = WITH_MEMS_MIC ? (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX)
                            : (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = SAMPLE_RATE,  // samplerate configured are the number of samples returned if channel=Left+Right, not frequency. So we need to multiply*2.
      .bits_per_sample = WITH_MEMS_MIC ? (i2s_bits_per_sample_t)(BYTES_X_SAMPLE * 8) : I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = WITH_MEMS_MIC ? I2S_COMM_FORMAT_STAND_I2S : I2S_COMM_FORMAT_STAND_MSB,  // I2S_COMM_FORMAT_STAND_MSB, I2S_COMM_FORMAT_I2S_MSB
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                                       // Interrupt level 1, default 0
      .dma_buf_count = 2,                                                                             // 8, //the length of an internal real DMA buffer shouldn’t be greater than 4092.
      .dma_buf_len = (AUDIO_DATA_OUT * OVERSAMPLING),                                                 // AUDIO_DATA_OUT*OVERSAMPLING, //these are the number of samples we will read from i2s_read
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0  //,//SAMPLE_RATE,
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
      .bck_io_num = WITH_MEMS_MIC ? PIN_MIC_I2S_SCK : I2S_PIN_NO_CHANGE,  // Sample f(Hz) (= sample f * 2)  on this pin (optional).
      .ws_io_num = WITH_MEMS_MIC ? PIN_MIC_I2S_WS : I2S_PIN_NO_CHANGE,    // Left/Right   (= sample f)      on this pin (optional).
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = WITH_MEMS_MIC ? PIN_MIC_I2S_SD : PIN_AUDIO_IN  // RTCIO_CHANNEL_4_GPIO_NUM // ADC1_CHANNEL_4_GPIO_NUM// RTCIO_CHANNEL_4_GPIO_NUM
  };
  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
    log_e("i2s_set_pin failed");
  }
  // init ADC pad
  if (!WITH_MEMS_MIC) {
    err = i2s_set_adc_mode(ADC_UNIT_1, PIN_AUDIO_IN);  // ADC1_CHANNEL_4
    if (err != ESP_OK) {
      log_e("set_adc_mode failed");
    }
    // //enable de ADC
    err = i2s_adc_enable(I2S_NUM_0);
  }
  // Create queue to communicate reader & drawer
  _xQueSendAudio2Drawer = xQueueCreate(1, sizeof(MsgAudio2Draw));
  _xQueSendFft2Led = xQueueCreate(1, sizeof(MsgAudio2Draw));
  // start task to read samples from I2S
  xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 9000, (void*)&_TaskParams, 4, &_readerTaskHandle, 0);  // 7000
  // xTaskCreate(vTaskReader, "ReaderTask", 6000, (void*)&_TaskParams, 4, &_readerTaskHandle); // 7000
  //  // xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 2048, (void*)&_TaskParams, 2, &_readerTaskHandle, 0);
  //  start task to draw screen
  xTaskCreatePinnedToCore(vTaskDrawer, "Draw Leds", 3000, (void*)&_TaskParams, 3, &_drawTaskHandle, 1);  // 3
  // xTaskCreate(vTaskDrawer, "Draw Leds", 3000, (void*)&_TaskParams, 3, &_drawTaskHandle);
  //     xTaskCreatePinnedToCore(vTaskShowLeds, "vTaskShowLeds", 2048, (void*)&_TaskParams, 2, &_drawTaskShowLeds, 1);
  //  // start task to draw leds
  //  // xTaskCreatePinnedToCore(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle, 0);
  //  //	xTaskCreate(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle);

  xTaskCreatePinnedToCore(vTaskWifiReconnect, "Wifi Reconnect", 5000, nullptr, 4, &_wifiReconnectTaskHandle, 1);  // 4096
  // xTaskCreate(vTaskWifiReconnect, "Wifi Reconnect", 5000, nullptr, 4, &_wifiReconnectTaskHandle); // 4096

  // xTaskCreate(vTaskRefrescarConsumElectricitat, "Refrescar Consum", 20000, nullptr, 2, &_refrescarConsumTaskHandle);
  //** xTaskCreatePinnedToCore(vTaskRefrescarConsumElectricitat, "Refrescar Consum", 15000, nullptr, 2, &_refrescarConsumTaskHandle, 1);
  xTaskCreatePinnedToCore(vTaskReceiveIR, "Receive IR", 3000, nullptr, 2, &_receiveIRTaskHandle, 1);

  xTaskCreatePinnedToCore(vTaskResetWhenHung, "Reset When Hung", 2048, nullptr, 2, &_resetWhenHungTaskHandle, 1);
}

// PLAY WITH MATRIX
//  #define TEMP_LEDS (16*32*2) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32
//  CRGBArray<TEMP_LEDS> _MyTempLeds;
//  uint16_t _indexPix = 0;
//  bool added = false;
//  CLEDController* _MyTempController = nullptr;
// END PLAY WITH MATRIX

void loop() {
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
    _ThePubSub.publish(TOPIC_HIWATER_IRDECO, Utils::string_format("%d", uxTaskGetStackHighWaterMark(_receiveIRTaskHandle)).c_str());
  }
}

// Updates a given preference value. Each Pref uses a concrete Global value.
void UpdatePref(Prefs thePref) {
  switch (thePref) {
    case Prefs::PR_INTENSITY:
      _ThePrefs.putUShort(PREF_INTENSITY, _MAX_MILLIS);
      break;
    case Prefs::PR_STYLE:
      _ThePrefs.putUChar(PREF_STYLE, (uint8_t)_TheDrawStyle);
      break;
    case Prefs::PR_GROUPMINS:
      _ThePrefs.putUShort(PREF_CURRENT_BY_MINUTES, _AgrupaConsumsPerMinuts);
      break;
    case Prefs::PR_PIANOMODE:
      _ThePrefs.putBool(PREF_PIANOMODE, _pianoMode);
      break;
    case Prefs::PR_NIGHTMODE:
      _ThePrefs.putBool(PREF_NIGHTMODE, _NightMode);
      break;
    case Prefs::PR_CUSTOM_HUE:
      _ThePrefs.putInt(PREF_CUSTOM_HUE, _TheDesiredHue);
      break;
    case Prefs::PR_DAYLIGHT_SAVING:
      _ThePrefs.putBool(PREF_DAYLIGHT_SAVING, _DaylightSaving);
      break;
  }
}

void SetNightMode(bool nightOn) {
  _NightMode = nightOn;
  if (nightOn) {
    _MAX_MILLIS = NIGHT_MILLIS;
    _TheDrawStyle = DRAW_STYLE::BARS_WITH_TOP;
  } else if (!nightOn) {
    _MAX_MILLIS = DEFAULT_MILLIS;
    _TheDrawStyle = DRAW_STYLE::VERT_FIRE;
  }
  FastLED.setBrightness(_MAX_MILLIS);
}

void ProcessStyleChange(DRAW_STYLE oldStyle, DRAW_STYLE newStyle) {
  if (oldStyle == DRAW_STYLE::CALC_MODE) {  // restore clock font
    _u8g2.setFont(u8g2_font_princess_tr);   // u8g2_font_oskool_tr);
    _StartedCalcMode = -1;                  // reset calcMode
  }
}

bool ChangeDrawStyle(DRAW_STYLE style, bool forceChange) {
  bool allowStyleChange = _TheDrawStyle != DRAW_STYLE::CALC_MODE || forceChange;

  if (allowStyleChange && _TheDrawStyle != style) {
    ProcessStyleChange(_TheDrawStyle, style);
    _TheDrawStyle = style;
    if (style != DRAW_STYLE::CALC_MODE) {
      UpdatePref(Prefs::PR_STYLE);
    } else {
      _LastCalcKeyPressed = millis();
    }

    if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
      _UpdateCurrentNow = true;
    }

    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
    return true;
  }
  return false;
}

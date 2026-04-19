#include <Arduino.h>

// include globals
#include "GlobalDefines.h"

// #include <FastLED.h>
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

#include "PowerBarsPanel.h"
// needs to be included after PowerBarsPanel
#include "FftPower.h"
#include "SharedUtils/OtaUpdater.h"
#include "SharedUtils\Utils.h"
// Global variables and functions. need the previous includes to work properly
#include "Global.h"

//----------------------
// Advanced declarations
//----------------------
/// @brief Sets default parameters for night mode on/off
/// @param nightOn
void SetNightMode(bool nightOn, bool persistStyle = true);
/// @brief Persists a given preference value.
void UpdatePref(Prefs thePref);
/// @brief Processes a change in the drawing style, restoring the required parameters.
void ProcessStyleChange(DRAW_STYLE oldStyle, DRAW_STYLE newStyle);
/// @brief Changes the drawing style. Calls ProcessStyleChange.
bool ChangeDrawStyle(DRAW_STYLE style, bool forceChange = false, bool persist = true);
/// @brief Applies queued style changes from DrawerTask frame boundary.
void ProcessPendingStyleChanges();
/// @brief Clamps a draw style to the supported range.
DRAW_STYLE ClampDrawStyle(int styleValue, DRAW_STYLE maxStyle = DRAW_STYLE::MAX_STYLE);
/// @brief Clamps bar grouping to a safe range.
uint8_t ClampBinGrouping(int value);
/// @brief Returns the maximum safe bar spacing for a grouping.
uint8_t GetMaxBarSpacingForGrouping(uint8_t grouping);
/// @brief Applies led intensity and optionally persists it.
bool ApplyIntensitySetting(int intensity, bool persist = true);
/// @brief Applies the selected base hue and optionally persists it.
bool ApplyDesiredHueSetting(int hue, bool persist = true);
/// @brief Applies piano mode and optionally persists it.
bool ApplyPianoModeSetting(bool enabled, bool persist = true);
/// @brief Applies bin grouping in memory.
bool ApplyBinGroupingSetting(int value);
/// @brief Applies bar spacing in memory.
bool ApplyBarSpacingSetting(int value);
/// @brief Applies bar alter draw in memory.
bool ApplyBarAlterDrawSetting(int value);
/// @brief Applies fading wave mode in memory.
bool ApplyFadingWaveModeSetting(bool enabled);
/// @brief Applies clock zebra mode in memory.
bool ApplyClockZebraModeSetting(bool enabled);
/// @brief Applies Shazam mode in memory only.
bool ApplyShazamModeSetting(bool enabled);
/// @brief Applies night mode and optionally persists it.
bool ApplyNightModeSetting(bool enabled, bool persist = true);
/// @brief Requests an ASAP recognition to the Shazam service.
void TriggerShazamRecognizeAsap();

#include "Effects.h"  //uses some of the vars declared in Global
#include "Tasks.h"    //tasks might call some of the functions declared above

void setup() {
  Serial.begin(115200);
  // wait for serial monitor to open
  while (!Serial);

  // pinMode(PIN_BASS_LED, OUTPUT);
  pinMode(PIN_MIC_I2S_SCK, OUTPUT);
  pinMode(PIN_MIC_I2S_WS, OUTPUT);
  pinMode(PIN_MIC_I2S_SD, INPUT);

#if defined(PANEL_SIZE_64x32)
  pinMode(PIN_DATA_LEDS1, OUTPUT);
  pinMode(PIN_DATA_LEDS2, OUTPUT);
  pinMode(PIN_DATA_LEDS3, OUTPUT);
  pinMode(PIN_DATA_LEDS4, OUTPUT);
  pinMode(PIN_AUDIO_IN, INPUT);

  FastLED.addLeds<WS2812B, PIN_DATA_LEDS1, GRB>(_TheLeds, 0, THE_PANEL_WIDTH * 8);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS2, GRB>(_TheLeds, THE_PANEL_WIDTH * 8, THE_PANEL_WIDTH * 8);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS3, GRB>(_TheLeds, THE_PANEL_WIDTH * 8 * 2, THE_PANEL_WIDTH * 8);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS4, GRB>(_TheLeds, THE_PANEL_WIDTH * 8 * 3, THE_PANEL_WIDTH * 8);
#elif defined(PANEL_SIZE_96x54)
  pinMode(PIN_DATA_LEDS1, OUTPUT);
  pinMode(PIN_DATA_LEDS2, OUTPUT);
  pinMode(PIN_DATA_LEDS3, OUTPUT);
  pinMode(PIN_DATA_LEDS4, OUTPUT);
  pinMode(PIN_DATA_LEDS5, OUTPUT);
  pinMode(PIN_DATA_LEDS6, OUTPUT);
  pinMode(PIN_DATA_LEDS7, OUTPUT);
  pinMode(PIN_DATA_LEDS8, OUTPUT);
  pinMode(PIN_DATA_LEDS9, OUTPUT);

  FastLED.addLeds<WS2812B, PIN_DATA_LEDS9, GRB>(_TheLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS8, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS7, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS6, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS5, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS4, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 5, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS3, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 6, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS2, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 7, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, PIN_DATA_LEDS1, GRB>(_TheLeds, NUM_LEDS_PER_STRIP * 8, NUM_LEDS_PER_STRIP);

  pinMode(PIN_ENABLE_LEVELCONV, OUTPUT);
  digitalWrite(PIN_ENABLE_LEVELCONV, HIGH);  // enable level converter
#endif

  _ThePanel.SetParams(&_TheLeds, &_TheMapping);
  InitLedIndexMap();

  _ThePrefs.begin("FlipaLeds", false);  // false=read+write
  _TheDrawStyle = (DRAW_STYLE)(_ThePrefs.getUChar(PREF_STYLE, DRAW_STYLE::DEFAULT_STYLE));
  _MAX_MILLIS = _ThePrefs.getUShort(PREF_INTENSITY, _MAX_MILLIS);
  _AgrupaConsumsPerMinuts = _ThePrefs.getUShort(PREF_CURRENT_BY_MINUTES, DEFAULT_CONSUM_PER_MINUTS);
  //_NightMode = _ThePrefs.getBool(PREF_NIGHTMODE, _NightMode); //night mode is always off at start
  _pianoMode = _ThePrefs.getBool(PREF_PIANOMODE, _pianoMode);
  _TheDesiredHue = _ThePrefs.getInt(PREF_CUSTOM_HUE, -1);
  //_TheDesiredHue = random8();  // DEBUUUUUUG

  _DaylightSaving = _ThePrefs.getBool(PREF_DAYLIGHT_SAVING, true);  // Horari d'estiu. uses _DaylightSaving.

  //_TheDrawStyle = DRAW_STYLE::VERT_FIRE;

  log_d("Loaded defaults: Style=%d Intensity=%d ConsumsPerMinut=%d NightMode=%d PianoMode=%d Hue=%d",
        (int)_TheDrawStyle, (int)_MAX_MILLIS, (int)_AgrupaConsumsPerMinuts, (int)_NightMode, (int)_pianoMode, (int)_TheDesiredHue);

  // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);
  // FastLED.setTemperature(Halogen);
  // FastLED.setMaxPowerInVoltsAndMilliamps(5, 20000);
  FastLED.setBrightness(_MAX_MILLIS);
  FastLED.setDither(DISABLE_DITHER);

  if (_NightMode) {
    SetNightMode(true);
  }

  log_d("Installing Driver...");
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,  // samplerate configured are the number of samples returned if channel=Left+Right, not frequency. So we need to multiply*2.
      .bits_per_sample = i2s_bits_per_sample_t::I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,  // I2S_COMM_FORMAT_STAND_MSB, I2S_COMM_FORMAT_I2S_MSB
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,           // Interrupt level 1, default 0
      .dma_buf_count = NUM_BUFFERS,                       // 8, //the length of an internal real DMA buffer shouldn’t be greater than 4092.
      .dma_buf_len = MIC_BUFFER_SIZE,                     // AUDIO_DATA_OUT*OVERSAMPLING, //these are the number of samples we will read from i2s_read
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0  //,//SAMPLE_RATE,
                       // .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
                       // .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
  };
  // install and start i2s driver
  auto err = i2s_driver_install(I2S_NUM_AUDIO, &i2s_config, _adc_i2s_event_queue_size, &_adc_i2s_event_queue);
  if (err != ESP_OK) {
    log_e("driver install failed");
  }
  // i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
  static const i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_MIC_I2S_SCK,  // Sample f(Hz) (= sample f * 2)  on this pin (optional).
      .ws_io_num = PIN_MIC_I2S_WS,    // Left/Right   (= sample f)      on this pin (optional).
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_MIC_I2S_SD  // RTCIO_CHANNEL_4_GPIO_NUM // ADC1_CHANNEL_4_GPIO_NUM// RTCIO_CHANNEL_4_GPIO_NUM
  };
  err = i2s_set_pin(I2S_NUM_AUDIO, &pin_config);
  if (err != ESP_OK) {
    log_e("i2s_set_pin failed");
  }
  // Create queue to communicate reader & drawer
  _xQueSendAudio2Drawer = xQueueCreate(1, sizeof(MsgAudio2Draw));
  _xQueStyleChange = xQueueCreate(1, sizeof(DrawStyleChangeRequest));
  //_xQueSendFft2Led = xQueueCreate(1, sizeof(MsgAudio2Draw));
  _xEventVertFireNewLine = xEventGroupCreate();
  _ThePanel.SetEventForBackgroundTasks(_xEventVertFireNewLine);  // set the event for background tasks

  // start task to read samples from I2S
  xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 6000, (void*)&_TaskParams, 4, &_readerTaskHandle, 0);  // 7000
  // xTaskCreate(vTaskReader, "ReaderTask", 6000, (void*)&_TaskParams, 4, &_readerTaskHandle); // 7000
  //  // xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 2048, (void*)&_TaskParams, 2, &_readerTaskHandle, 0);
  //  start task to draw screen
  xTaskCreatePinnedToCore(vTaskDrawer, "Draw Leds", 3000, (void*)&_TaskParams, 3, &_drawTaskHandle, 1);  // 3
  // xTaskCreate(vTaskDrawer, "Draw Leds", 3000, (void*)&_TaskParams, 3, &_drawTaskHandle);
  //     xTaskCreatePinnedToCore(vTaskShowLeds, "vTaskShowLeds", 2048, (void*)&_TaskParams, 2, &_drawTaskShowLeds, 1);
  //  // start task to draw leds
  //  // xTaskCreatePinnedToCore(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle, 0);
  //  //	xTaskCreate(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle);

  xTaskCreatePinnedToCore(vTaskWifiReconnect, "Wifi Reconnect", 5500, nullptr, 4, &_wifiReconnectTaskHandle, 1);  // 4096
  // xTaskCreate(vTaskWifiReconnect, "Wifi Reconnect", 5000, nullptr, 4, &_wifiReconnectTaskHandle); // 4096

  // xTaskCreate(vTaskRefrescarConsumElectricitat, "Refrescar Consum", 20000, nullptr, 2, &_refrescarConsumTaskHandle);
  //** xTaskCreatePinnedToCore(vTaskRefrescarConsumElectricitat, "Refrescar Consum", 15000, nullptr, 2, &_refrescarConsumTaskHandle, 1);
  xTaskCreatePinnedToCore(vTaskReceiveIR, "Receive IR", 2500, nullptr, 2, &_receiveIRTaskHandle, 1);

  xTaskCreate(vTaskVertFire, "Vertical Fire", 2500, (void*)_ThePanel.GetAuxLeds(), 2, &_vertFireTaskHandle);

  ////  xTaskCreatePinnedToCore(vTaskResetWhenHung, "Reset When Hung", 2048, nullptr, 2, &_resetWhenHungTaskHandle, 1);
}

// PLAY WITH MATRIX
//  #define TEMP_LEDS (16*32*2) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32
//  CRGBArray<TEMP_LEDS> _MyTempLeds;
//  uint16_t _indexPix = 0;
//  bool added = false;
//  CLEDController* _MyTempController = nullptr;
// END PLAY WITH MATRIX

constexpr uint32_t ONE_HOUR_IN_MILLIS = 60 * 60 * 1000;
constexpr uint32_t HALF_HOUR_IN_MILLIS = 30 * 60 * 1000;
void loop() {
  if (millis() - _DesiredHueLastSet > HALF_HOUR_IN_MILLIS) {
    _DesiredHueLastSet = millis();

    struct tm timeinfo;
    getLocalTime(&timeinfo);
    if (timeinfo.tm_hour >= 21 && timeinfo.tm_min >= 30 && !_NightMode) {
      SetNightMode(true);  // set night mode on
      SendDebugMessage("Night mode ON");
    } else if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 21 && _NightMode) {
      SetNightMode(false);  // set night mode off
      SendDebugMessage("Night mode OFF");
    } else if (!_NightMode) {
      _TheDesiredHue = _TheFrameNumber % 255;  // random8(); //randomize hue every hour
      UpdatePref(Prefs::PR_CUSTOM_HUE);
      log_d("Randomized Hue=%d", _TheDesiredHue);
      SendDebugMessage(Utils::string_format("Randomized Hue=%d", _TheDesiredHue).c_str());
      _binGrouping = (_TheFrameNumber % 5) + 2;  // randomize binGrouping every hour
      log_d("Randomized binGrouping=%d", _binGrouping);
      SendDebugMessage(Utils::string_format("Randomized binGrouping=%d", _binGrouping).c_str());
      _barSpacing = (_TheFrameNumber % (_binGrouping - 1)) + 1;  // randomize barSpacing every hour
      log_d("Randomized barSpacing=%d", _barSpacing);
      SendDebugMessage(Utils::string_format("Randomized barSpacing=%d", _barSpacing).c_str());
      _barAlterDraw = (ALTERDRAW)((_TheFrameNumber % 2) + 1);  // randomize barAlterDraw every hour
      log_d("Randomized barAlterDraw=%d", (int)_barAlterDraw);
      SendDebugMessage(Utils::string_format("Randomized barAlterDraw=%d", (int)_barAlterDraw).c_str());

      if (_TheDrawStyle != DRAW_STYLE::CALC_MODE) {
        auto newStyle = (DRAW_STYLE)(((_TheFrameNumber % (int)DRAW_STYLE::ANALOG_CLOCK)) + 1);  // randomize style every hour
        if (newStyle == DRAW_STYLE::HORIZ_FIRE) {
          newStyle = DRAW_STYLE::BARS_WITH_TOP;  // avoid audiostreamer mode
        }
        log_d("Randomized DrawStyle=%d", (int)newStyle);
        SendDebugMessage(Utils::string_format("Randomized DrawStyle=%d", (int)newStyle).c_str());
        ChangeDrawStyle(newStyle, false);
      }
#if defined(PANEL_SIZE_96x54)
      _FadingWaveMode = _TheFrameNumber % 2 == 0;
      log_d("Randomized FadingWaveMode=%d", _FadingWaveMode ? 1 : 0);
      SendDebugMessage(Utils::string_format("Randomized FadingWaveMode=%d", _FadingWaveMode ? 1 : 0).c_str());
#endif
    }
  }
  delay(10000);
  log_d("Main: Free Heap=%d largest_block=%d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  if (_DebugMode && _ThePubSub.connected()) {
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

void SetNightMode(bool nightOn, bool persistStyle) {
  _NightMode = nightOn;
  if (nightOn) {
    _MAX_MILLIS = NIGHT_MILLIS;
    ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true, persistStyle);
    _barAlterDraw = ALTERDRAW::ODD;
    _FadingWaveMode = false;  // no volem fer fading en aquest mode
    _barSpacing = 4;
    _binGrouping = 5;
    _ShazamSongs = false;  // no volem detectar cançons en mode nit
  } else if (!nightOn) {
    _MAX_MILLIS = DEFAULT_MILLIS;
    ChangeDrawStyle(DRAW_STYLE::VERT_FIRE, true, persistStyle);
    _ShazamSongs = true;  // volem detectar cançons en mode dia
  }
  FastLED.setBrightness(_MAX_MILLIS);
}

void ProcessStyleChange(DRAW_STYLE oldStyle, DRAW_STYLE newStyle) {
  if (oldStyle == DRAW_STYLE::CALC_MODE) {  // restore clock font
    _u8g2.setFont(u8g2_font_princess_tr);   // u8g2_font_oskool_tr);
    _StartedCalcMode = -1;                  // reset calcMode
  } else if (oldStyle == DRAW_STYLE::HORIZ_FIRE) {
    if (_pBuffAudio2Send) {
      free(_pBuffAudio2Send);
      _pBuffAudio2Send = nullptr;
    }
  }
}

namespace {
bool ApplyDrawStyleChange(DRAW_STYLE style, bool forceChange, bool persist) {
  bool allowStyleChange = _TheDrawStyle != DRAW_STYLE::CALC_MODE || forceChange;

  if (allowStyleChange && _TheDrawStyle != style) {
    ProcessStyleChange(_TheDrawStyle, style);
    _TheDrawStyle = style;
    if (style != DRAW_STYLE::CALC_MODE && persist) {
      UpdatePref(Prefs::PR_STYLE);
    } else {
      _LastCalcKeyPressed = millis();
    }

    if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
      _UpdateCurrentNow = true;
    }

    SendDebugMessage(Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str());
    return true;
  }

  if (allowStyleChange && persist && style != DRAW_STYLE::CALC_MODE) {
    UpdatePref(Prefs::PR_STYLE);
  }
  return false;
}
}  // namespace

DRAW_STYLE ClampDrawStyle(int styleValue, DRAW_STYLE maxStyle) {
  return (DRAW_STYLE)max(min(styleValue, (int)maxStyle), (int)DRAW_STYLE::BARS_WITH_TOP);
}

uint8_t ClampBinGrouping(int value) {
  return (uint8_t)max(min(value, 8), 1);
}

uint8_t GetMaxBarSpacingForGrouping(uint8_t grouping) {
  return grouping > 1 ? (grouping - 1) : 0;
}

bool ApplyIntensitySetting(int intensity, bool persist) {
  uint16_t newIntensity = (uint16_t)max(min(intensity, 255), 0);
  if (_MAX_MILLIS == newIntensity) {
    if (persist) {
      UpdatePref(Prefs::PR_INTENSITY);
    }
    return false;
  }

  _MAX_MILLIS = newIntensity;
  FastLED.setBrightness(_MAX_MILLIS);
  if (persist) {
    UpdatePref(Prefs::PR_INTENSITY);
  }
  SendDebugMessage(Utils::string_format("Updated intensity=%d", _MAX_MILLIS).c_str());
  return true;
}

bool ApplyDesiredHueSetting(int hue, bool persist) {
  int16_t newHue = (int16_t)max(min(hue, 255), -1);
  if (_TheDesiredHue == newHue) {
    if (persist) {
      UpdatePref(Prefs::PR_CUSTOM_HUE);
    }
    return false;
  }

  _TheDesiredHue = newHue;
  _DesiredHueLastSet = millis();
  if (persist) {
    UpdatePref(Prefs::PR_CUSTOM_HUE);
  }
  SendDebugMessage(Utils::string_format("Updated Hue=%d", _TheDesiredHue).c_str());
  return true;
}

bool ApplyPianoModeSetting(bool enabled, bool persist) {
  if (_pianoMode == enabled) {
    if (persist) {
      UpdatePref(Prefs::PR_PIANOMODE);
    }
    return false;
  }

  _pianoMode = enabled;
  if (persist) {
    UpdatePref(Prefs::PR_PIANOMODE);
  }
  SendDebugMessage(Utils::string_format("Piano Mode %s!!", _pianoMode ? "ON" : "OFF").c_str());
  return true;
}

bool ApplyBinGroupingSetting(int value) {
  uint8_t newGrouping = ClampBinGrouping(value);
  bool changed = _binGrouping != newGrouping;

  _binGrouping = newGrouping;
  uint8_t maxSpacing = GetMaxBarSpacingForGrouping(_binGrouping);
  if (_barSpacing > maxSpacing) {
    _barSpacing = maxSpacing;
    changed = true;
  }

  if (changed) {
    SendDebugMessage(Utils::string_format("BinGrouping=%d", (int)_binGrouping).c_str());
  }
  return changed;
}

bool ApplyBarSpacingSetting(int value) {
  uint8_t maxSpacing = GetMaxBarSpacingForGrouping(_binGrouping);
  uint8_t newSpacing = (uint8_t)max(min(value, (int)maxSpacing), 0);
  if (_barSpacing == newSpacing) {
    return false;
  }

  _barSpacing = newSpacing;
  SendDebugMessage(Utils::string_format("BarSpacing=%d", (int)_barSpacing).c_str());
  return true;
}

bool ApplyBarAlterDrawSetting(int value) {
  ALTERDRAW newAlter = (ALTERDRAW)max(min(value, (int)ALTERDRAW::ALTERNATE), (int)ALTERDRAW::NO_ALTER);
  if (_barAlterDraw == newAlter) {
    return false;
  }

  _barAlterDraw = newAlter;
  SendDebugMessage(Utils::string_format("BarAlterDraw=%d", (int)_barAlterDraw).c_str());
  return true;
}

bool ApplyFadingWaveModeSetting(bool enabled) {
  if (_FadingWaveMode == enabled) {
    return false;
  }

  _FadingWaveMode = enabled;
  SendDebugMessage(Utils::string_format("Fading Wave Mode %s!!", _FadingWaveMode ? "ON" : "OFF").c_str());
  return true;
}

bool ApplyShazamModeSetting(bool enabled) {
  if (_ShazamSongs == enabled) {
    return false;
  }

  _ShazamSongs = enabled;
  if (enabled) {
    _ShazamActivationTime = millis();
    SendDebugMessage("Shazam Mode ON!!");
  } else {
    _DisplayAsapIndicator = false;
    SendDebugMessage("Shazam Mode OFF!!");
  }
  return true;
}

bool ApplyNightModeSetting(bool enabled, bool persist) {
  if (_NightMode == enabled) {
    if (persist) {
      UpdatePref(Prefs::PR_NIGHTMODE);
    }
    return false;
  }

  SetNightMode(enabled, persist);
  if (persist) {
    UpdatePref(Prefs::PR_NIGHTMODE);
  }
  SendDebugMessage(Utils::string_format("Updated Nightmode=%d", (int)enabled).c_str());
  return true;
}

void TriggerShazamRecognizeAsap() {
  ApplyShazamModeSetting(true);
  SendDebugMessage("Shazam Mode ON (ASAP)!!");
  _DisplayAsapIndicator = true;
  _AsapDetectionTime = millis();
  _ThePubSub.publish(TOPIC_RECOGNIZE_ASAP, "1", false);
}

void ProcessPendingStyleChanges() {
  if (!_xQueStyleChange) {
    return;
  }
  DrawStyleChangeRequest req;
  while (xQueueReceive(_xQueStyleChange, &req, 0) == pdPASS) {
    ApplyDrawStyleChange(req.style, req.forceChange, req.persist);
  }
}

bool ChangeDrawStyle(DRAW_STYLE style, bool forceChange, bool persist) {
  if (!_xQueStyleChange) {
    return ApplyDrawStyleChange(style, forceChange, persist);
  }
  DrawStyleChangeRequest req;
  req.style = style;
  req.forceChange = forceChange;
  req.persist = persist;
  return xQueueOverwrite(_xQueStyleChange, &req) == pdPASS;
}

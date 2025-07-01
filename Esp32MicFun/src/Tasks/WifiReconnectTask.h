// Advanced declarations
/// @brief  Connects to the MQTT broquer if not connected.
void Connect2MQTT();
/// @brief  PubSubClient callback for received messages
void PubSubCallback(char* pTopic, uint8_t* pData, unsigned int dalaLength);
/// @brief  Processes the received http payload in csv form and populates the global VisualCurrentConsumption structures
void ProcessCurrentPayload(std::string& thePayload);
/// @brief Configures the NTP server. In this project the timezone and offsets are hardcoded to suit my needs
void ConfigureNTP();

/// @brief This task will try to reconnect to the wifi network every RETRY_WIFI_EVERY_SECS seconds
/// Will also try to reconnect to the MQTT server and to the NTP server to maintain the hour updated
/// Will also try to get the current consumption from the server when the drawing style is set to VISUAL_CURRENT
/// @param pvParameters
void vTaskWifiReconnect(void* pvParameters) {
  bool reconnected = false;

  auto lastCheck = millis();
  // WiFiClientFixed httpWifiClient;
  // HTTPClient theHttpClient;
  WiFiClientSecure* __httpWifiClient = new WiFiClientSecure();
  HTTPClient* __theHttpClient = new HTTPClient();
  std::string theUrl;
  time_t now;

  sleep(20);  // wait a bit...

  _OTA.Setup("MicFun", "", OTA_PORT);
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
        auto err = WiFi.begin(WIFI_SSID, WIFI_PASS);  // FROM mykeys.h
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
      _ThePubSub.loop();  // allow the pubsubclient to process incoming messages
      _OTA.Process();
      // _TheNTPClient.update();
      if (!_ThePubSub.connected()) {
        _ThePubSub.setBufferSize((THUMBNAIL_WIDTH * THE_PANEL_WIDTH) + 128);  // 1024
        Connect2MQTT();
        if (_ThePubSub.connected()) {
          _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("MicFun connected with IP=[%s]", WiFi.localIP().toString().c_str()).c_str(), false);
          _ThePubSub.publish(TOPIC_LASTIP, Utils::string_format("%s", WiFi.localIP().toString().c_str()).c_str(), true);
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

void ConfigureNTP() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  // Is time set? If not, tm_year will be (1970 - 1900).
  //    if (timeinfo.tm_year < (2020 - 1900)) {
  //        log_i("Time is not set yet. Connecting to WiFi and getting time over NTP.");
  if (WiFi.isConnected()) {                    // if its not connected, the ntp server might crash (bug, probably solved already)
    if (_DaylightSaving) {                     // estiu
      configTime(3600, 3600, "pool.ntp.org");  // 3600, 0 --> horari hivern  3600, 3600 --> horari estiu
    } else {                                   // hivern
      configTime(3600, 0, "pool.ntp.org");     // 3600, 0 --> horari hivern  3600, 3600 --> horari estiu
    }
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

void Connect2MQTT() {
  if (!_ThePubSub.connected()) {
    _ThePubSub.setClient(_TheWifi);
    _ThePubSub.setServer(MQTT_BROKER, MQTT_PORT);
    _ThePubSub.setCallback(PubSubCallback);
    //		String s = WiFi.macAddress());
    if (!_ThePubSub.connect((String("ESP32_Espectrometer") + WiFi.macAddress()[0]).c_str())) {
      log_e("ERROR!! PubSubClient was not able to connect to PiRuter!!");
    } else {  // Subscribe to the feeds
      _LastMqttReconnect = millis();
      log_i("PubSubClient connected to PiRuter MQTT broker!!");
      _ThePubSub.publish(TOPIC_DEBUG, "PubSubClient connected to PiRuter MQTT broker!!", true);

      if (!_ThePubSub.subscribe(TOPIC_INTENSITY)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_INTENSITY);
      }
      if (!_ThePubSub.subscribe(TOPIC_DELAYFRAME)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_DELAYFRAME);
      }
      if (!_ThePubSub.subscribe(TOPIC_STYLE)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_STYLE);
      }
      if (!_ThePubSub.subscribe(TOPIC_RESET)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_RESET);
      }
      if (!_ThePubSub.subscribe(TOPIC_NIGHTMODE)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_NIGHTMODE);
      }
      if (!_ThePubSub.subscribe(TOPIC_GROUPMINUTS)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_GROUPMINUTS);
      }
      if (!_ThePubSub.subscribe(TOPIC_HORARI_ESTIU)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_GROUPMINUTS);
      }
      if (!_ThePubSub.subscribe(TOPIC_SONG_NAME)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_SONG_NAME);
      }
      if (!_ThePubSub.subscribe(TOPIC_THUMBNAIL)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_THUMBNAIL);
      }
      if (!_ThePubSub.subscribe(TOPIC_SHAZAM_MODE)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_SHAZAM_MODE);
      }

      // if (!_ThePubSub.subscribe(TOPIC_FPS)) {
      //     log_e("ERROR!! PubSubClient was not able to suibscribe to [%s]", TOPIC_FPS);
      // }
    }
  }
}

#if defined(PANEL_SIZE_96x48)
const uint8_t gamma_1_8_table[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2,
    2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6,
    6, 6, 7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11, 11, 12, 12,
    13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 21,
    21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31,
    32, 32, 33, 34, 35, 35, 36, 37, 38, 38, 39, 40, 41, 41, 42, 43,
    44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90,
    91, 92, 93, 95, 96, 97, 98, 99, 100, 102, 103, 104, 105, 107, 108, 109,
    110, 111, 113, 114, 115, 116, 118, 119, 120, 122, 123, 124, 126, 127, 128, 129,
    131, 132, 134, 135, 136, 138, 139, 140, 142, 143, 145, 146, 147, 149, 150, 152,
    153, 154, 156, 157, 159, 160, 162, 163, 165, 166, 168, 169, 171, 172, 174, 175,
    177, 178, 180, 181, 183, 184, 186, 188, 189, 191, 192, 194, 195, 197, 199, 200,
    202, 204, 205, 207, 208, 210, 212, 213, 215, 217, 218, 220, 222, 224, 225, 227,
    229, 230, 232, 234, 236, 237, 239, 241, 243, 244, 246, 248, 250, 251, 253, 255};
#else
const uint8_t _gamma8[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
    37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
    51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
    69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
    90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
    115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};
#endif
void DecodeThumbnail(uint8_t* pData, uint16_t dataLenght) {
  _ThumbnailImg.resize(dataLenght / 2);  // 2 bytes per pixel (RGB565)
  for (uint16_t i = 0; i < dataLenght / 2; i++) {
    uint16_t pixel = (pData[i * 2] << 8) | pData[i * 2 + 1];
    uint8_t r = (uint8_t)((pixel >> 11) & 0x1F) << 3;
    uint8_t g = (uint8_t)((pixel >> 5) & 0x3F) << 2;
    uint8_t b = (uint8_t)(pixel & 0x1F) << 3;
    if (r > 2) {
      r -= 2;
    }
    if (g > 2) {
      g -= 2;
    }
    if (b > 2) {
      b -= 2;
    }
#if defined(PANEL_SIZE_96x48)
    //    _ThumbnailImg[i] = CRGB(r, g, b);  // Convert to RGB888 //no adjust gamma
    _ThumbnailImg[i] = CRGB(gamma_1_8_table[r], gamma_1_8_table[g], gamma_1_8_table[b]);  // adjust gamma

#else
    _ThumbnailImg[i] = CRGB(_gamma8[r], _gamma8[g], _gamma8[b]);  // adjust gamma
#endif

    // to compensate the RGB565 to RGB888 conversion
    //    // Extreure components
    //  uint8_t r = (packed >> 11) & 0x1F;
    //  uint8_t g = (packed >> 5) & 0x3F;
    //  uint8_t b = packed & 0x1F;

    //   // Escalar a 8 bits
    // r = (r << 3) | (r >> 2);
    // g = (g << 2) | (g >> 4);
    // b = (b << 3) | (b >> 2);
  }
  _ThumbnailReady = true;
  log_i("Thumbnail image decoded. Size=%d pixels", _ThumbnailImg.size());
}

void PubSubCallback(char* pTopic, uint8_t* pData, unsigned int dataLenght) {
  std::string theTopic(pTopic);
  std::string theMsg;

  if (theTopic.find(TOPIC_THUMBNAIL) == std::string::npos) {
    for (uint16_t i = 0; i < dataLenght; i++) {
      theMsg.push_back((char)pData[i]);
    }
    log_v("Received message from [%s]: [%s]", theTopic.c_str(), theMsg.c_str());
  } else {
    log_v("Received message from [%s]. Data len=%dbytes", theTopic.c_str(), dataLenght);
  }

  if (millis() - _LastMqttReconnect < MQTT_RECONNECT_IGNORE_MSG_MS) {
    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Ignoring MqttMSG [%s][%s]", theTopic.c_str(), theMsg.c_str()).c_str(), true);
    return;
  }

  if (theTopic.find(TOPIC_INTENSITY) != std::string::npos) {
    auto origIntensity = _MAX_MILLIS;
    auto newIntensity = min(std::atoi(theMsg.c_str()), 255);  //(int)MAX_MILLIS);
    _MAX_MILLIS = newIntensity;

    if (newIntensity != origIntensity) {
      // FastLED.setMaxPowerInVoltsAndMilliamps(5, _MAX_MILLIS);
      FastLED.setBrightness(_MAX_MILLIS);
      UpdatePref(Prefs::PR_INTENSITY);

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
    ChangeDrawStyle((DRAW_STYLE)max(min(std::atoi(theMsg.c_str()), (int)DRAW_STYLE::MAX_STYLE), 1));
  }
  if (theTopic.find(TOPIC_RESET) != std::string::npos) {
    bool resetNow = std::atoi(theMsg.c_str()) != 0;
    if (resetNow) {
      ESP.restart();
    }
  }
  if (theTopic.find(TOPIC_NIGHTMODE) != std::string::npos) {
    byte nightOn = std::atoi(theMsg.c_str()) != 0;
    _ThePrefs.putBool(PREF_NIGHTMODE, nightOn);
    auto oldStyle = _TheDrawStyle;
    SetNightMode(nightOn);
    UpdatePref(Prefs::PR_STYLE);
    UpdatePref(Prefs::PR_NIGHTMODE);
    ProcessStyleChange(oldStyle, _TheDrawStyle);
    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated Nightmode=%d", (int)nightOn).c_str(), true);
  }
  if (theTopic.find(TOPIC_GROUPMINUTS) != std::string::npos) {
    int minuts = std::atoi(theMsg.c_str());
    _AgrupaConsumsPerMinuts = (uint16_t)max(min(minuts, (int)60), 1);
    UpdatePref(Prefs::PR_GROUPMINS);

    _UpdateCurrentNow = true;
    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Electricitat agrupada per [%d] minuts", (int)_AgrupaConsumsPerMinuts).c_str(), true);
  }
  if (theTopic.find(TOPIC_HORARI_ESTIU) != std::string::npos) {
    _DaylightSaving = std::atoi(theMsg.c_str()) != 0;
    UpdatePref(Prefs::PR_DAYLIGHT_SAVING);
    ConfigureNTP();
    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated Horari Estiu=%d", (int)_DaylightSaving).c_str(), true);
  }
  if (theTopic.find(TOPIC_SONG_NAME) != std::string::npos) {
    _DetectedSongName = theMsg;
    _ShazamSongs = true;  // force to show the song name
    _DisplayAsapIndicator = false;
    if (_DetectedSongName.length() < 4) {
      _DetectedSongName = "Can\xE7\xF3 desconeguda";
    }
    _LastSongDetectionTime = millis();
    _LastSongDisplayTime = millis() - 20000;  // forcem actualitzar el nom de la cançó asap
    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Detected SongName=[%s]", _DetectedSongName.c_str()).c_str(), true);
  }
  if (theTopic.find(TOPIC_THUMBNAIL) != std::string::npos) {
    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Detected Thumbnail. Datalen=%d bytes", dataLenght).c_str(), true);
    DecodeThumbnail(pData, dataLenght);
    if (_TheDrawStyle != DRAW_STYLE::DRAW_THUMBNAIL) {
      _ThumbnailPrevStyle = _TheDrawStyle;         // remember the style when the thumbnail was received
      _TheDrawStyle = DRAW_STYLE::DRAW_THUMBNAIL;  // force the thumbnail to be drawn
    }
    _TimeThumbnailReceived = millis();
  }
  if (theTopic.find(TOPIC_SHAZAM_MODE) != std::string::npos) {
    _ShazamSongs = true;
    _ThePubSub.publish(TOPIC_DEBUG, "Shazam Mode ON!!", false);
    _ShazamActivationTime = millis();
  }
  // _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Received Topic=[%s] Msg=[%s]", theTopic.c_str(), theMsg.c_str()).c_str(), true);
}

void ProcessCurrentPayload(std::string& theCsvPayload) {
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

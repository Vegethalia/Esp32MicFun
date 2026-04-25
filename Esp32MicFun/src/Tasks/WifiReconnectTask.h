#include "ConfigWebServer.h"

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
  //WiFiClientSecure* __httpWifiClient = new WiFiClientSecure();
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
      ConfigWebServer::SetupIfNeeded();
      ConfigWebServer::HandleClient();
      _ThePubSub.loop();  // allow the pubsubclient to process incoming messages
      _OTA.Process();
      // _TheNTPClient.update();
      if (!_ThePubSub.connected()) {
        _ThePubSub.setBufferSize((THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 2) + 128);  // 1024
        Connect2MQTT();
        if (_ThePubSub.connected()) {
          SendDebugMessage(Utils::string_format("MicFun connected with IP=[%s]", WiFi.localIP().toString().c_str()).c_str());
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
      SendDebugMessage(Utils::string_format("ConsumURL=[%s]", theUrl.c_str()).c_str());

      //__httpWifiClient->setInsecure();

      __theHttpClient->setConnectTimeout(2000);
      //__theHttpClient->setReuse(false);
      if (__theHttpClient->begin(theUrl.c_str())) {  // begin(*__httpWifiClient, theUrl.c_str())) {
        __theHttpClient->setConnectTimeout(2000);
        __theHttpClient->addHeader("Accept", "*/*");
        int httpResponse = __theHttpClient->GET();
        if (httpResponse > 0) {
          std::string thePayload = __theHttpClient->getString().c_str();
          SendDebugMessage(Utils::string_format("HTTP response [%d] length=[%d]", httpResponse, thePayload.length()).c_str());
          if (httpResponse == HTTP_CODE_OK) {
            // log_i("Payload=[%s]", thePayload.c_str());
            ProcessCurrentPayload(thePayload);
          }
        } else {
          SendDebugMessage(Utils::string_format("ConsumURL=[%s]", theUrl.c_str()).c_str());
          SendDebugMessage(Utils::string_format("GET returned code [%d]", httpResponse).c_str());
        }
      } else {
        SendDebugMessage(Utils::string_format("Begin returned false :(").c_str());
      }
      __theHttpClient->end();
    }
    delay(50);
  }
}

// NTP servers
const char* _ntpServer1 = "pool.ntp.org";
const char* _ntpServer2 = "time.google.com";
const char* _ntpServer3 = "time.nist.gov";

// TZ per a Espanya peninsular i Balears:
// CET-1 (UTC+1) + CEST (DST) amb transicions:
// - M3.5.0/2  = últim diumenge de març a les 02:00
// - M10.5.0/3 = últim diumenge d’octubre a les 03:00
const char* _TZ_CATALUNYA = "CET-1CEST,M3.5.0/2,M10.5.0/3";

void ConfigureNTP() {
  time_t now;
  struct tm timeinfo;

  // Estableix la zona horària POSIX (activa canvi automàtic d’estiu/hivern)
  setenv("TZ", _TZ_CATALUNYA, 1);
  tzset();

  // time(&now);
  // localtime_r(&now, &timeinfo);

  // Is time set? If not, tm_year will be (1970 - 1900).
  //    if (timeinfo.tm_year < (2020 - 1900)) {
  //        log_i("Time is not set yet. Connecting to WiFi and getting time over NTP.");
  if (WiFi.isConnected()) {  // if its not connected, the ntp server might crash (bug, probably solved already)
                             // if (_DaylightSaving) {                     // estiu
                             //   configTime(3600, 3600, "pool.ntp.org");  // 3600, 0 --> horari hivern  3600, 3600 --> horari estiu
                             // } else {                                   // hivern
                             //   configTime(3600, 0, "pool.ntp.org");     // 3600, 0 --> horari hivern  3600, 3600 --> horari estiu
                             // }
                             // Demana hora al NTP(no apliquem offsets aquí; ja ho fa la TZ)
    configTime(0, 0, _ntpServer1, _ntpServer2, _ntpServer3);

    sntp_set_sync_interval(60000);
    sntp_restart();
    sleep(1);  // wait a bit to let sntp get the time

    setenv("TZ", _TZ_CATALUNYA, 1);
    tzset();
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
      SendDebugMessage("PubSubClient connected to PiRuter MQTT broker!!");

      if (!_ThePubSub.subscribe(TOPIC_INTENSITY)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_INTENSITY);
      }
      if (!_ThePubSub.subscribe(TOPIC_STYLE)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_STYLE);
      }
      if (!_ThePubSub.subscribe(TOPIC_BASEHUE)) {
        // log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_BASEHUE);
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
      if (!_ThePubSub.subscribe(TOPIC_PIANO_MODE)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_PIANO_MODE);
      }
      if (!_ThePubSub.subscribe(TOPIC_DEBUG_MODE)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_DEBUG_MODE);
      }
      if (!_ThePubSub.subscribe(TOPIC_FADINGWAVE_MODE)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_FADINGWAVE_MODE);
      }
      if (!_ThePubSub.subscribe(TOPIC_TEMP_TERRASSA)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_TEMP_TERRASSA);
      }
      if (!_ThePubSub.subscribe(TOPIC_TEMP_INTERIOR)) {
        //   log_e("ERROR!! PubSubClient was not able to subscribe to [%s]", TOPIC_TEMP_INTERIOR);
      }
    }
  }
}

#include "../ColorUtils.h"

void DecodeThumbnail(uint8_t* pData, uint16_t dataLenght) {
  const uint16_t expectedBytes = THUMBNAIL_HEIGHT * THUMBNAIL_WIDTH * 2;
  if (dataLenght < expectedBytes) {
    log_w("Thumbnail too small: got %d, expected %d bytes. Discarding.", dataLenght, expectedBytes);
    _ThumbnailReady = false;
    return;
  }
  // Allocate once; reuse the same buffer for subsequent thumbnails (size is always fixed)
  if (!_ThumbnailRaw) {
    _ThumbnailRaw = (uint8_t*)malloc(expectedBytes);
    if (!_ThumbnailRaw) {
      log_e("malloc failed for thumbnail (%d bytes). Not enough heap.", expectedBytes);
      _ThumbnailReady = false;
      return;
    }
  }
  memcpy(_ThumbnailRaw, pData, expectedBytes);
  _ThumbnailReady = true;
  log_i("Thumbnail stored. %d bytes raw RGB565 (free heap: %d)", expectedBytes, ESP.getFreeHeap());
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
    SendDebugMessage(Utils::string_format("Ignoring MqttMSG [%s][%s]", theTopic.c_str(), theMsg.c_str()).c_str());
    return;
  }

  if (theTopic.find(TOPIC_INTENSITY) != std::string::npos) {
    ApplyIntensitySetting(std::atoi(theMsg.c_str()));
  } else if (theTopic.find(TOPIC_STYLE) != std::string::npos) {
    ChangeDrawStyle(ClampDrawStyle(std::atoi(theMsg.c_str())));
  } else if (theTopic.find(TOPIC_BASEHUE) != std::string::npos) {
    ApplyDesiredHueSetting(std::atoi(theMsg.c_str()));
  } else if (theTopic.find(TOPIC_RESET) != std::string::npos) {
    bool resetNow = std::atoi(theMsg.c_str()) != 0;
    if (resetNow) {
      ESP.restart();
    }
  } else if (theTopic.find(TOPIC_NIGHTMODE) != std::string::npos) {
    ApplyNightModeSetting(std::atoi(theMsg.c_str()) != 0);
  } else if (theTopic.find(TOPIC_GROUPMINUTS) != std::string::npos) {
    int minuts = std::atoi(theMsg.c_str());
    _AgrupaConsumsPerMinuts = (uint16_t)max(min(minuts, (int)60), 1);
    UpdatePref(Prefs::PR_GROUPMINS);

    _UpdateCurrentNow = true;
    SendDebugMessage(Utils::string_format("Electricitat agrupada per [%d] minuts", (int)_AgrupaConsumsPerMinuts).c_str());
  } else if (theTopic.find(TOPIC_HORARI_ESTIU) != std::string::npos) {
    _DaylightSaving = std::atoi(theMsg.c_str()) != 0;
    UpdatePref(Prefs::PR_DAYLIGHT_SAVING);
    ConfigureNTP();
    SendDebugMessage(Utils::string_format("Updated Horari Estiu=%d", (int)_DaylightSaving).c_str());
  } else if (theTopic.find(TOPIC_SONG_NAME) != std::string::npos) {
    char artworkUrl[300] = {};
    auto sep = theMsg.find("@@");
    if (sep != std::string::npos) {
      _DetectedSongName = theMsg.substr(0, sep);
      strncpy(artworkUrl, theMsg.c_str() + sep + 2, sizeof(artworkUrl) - 1);
      // Trim trailing whitespace/newlines that some backends append
      size_t urlLen = strlen(artworkUrl);
      if (urlLen > 0) {
        char* end = artworkUrl + urlLen - 1;
        while (end > artworkUrl && (*end == '\n' || *end == '\r' || *end == ' ')) *end-- = '\0';
      }
    } else {
      _DetectedSongName = theMsg;
    }
    _ShazamSongs = true;  // force to show the song name
    _DisplayAsapIndicator = false;
    _SongDesconeguda = false;
    if (_DetectedSongName.length() < 4) {
      _DetectedSongName = "Can\xE7\xF3 desconeguda";
      _SongDesconeguda = true;  // force to show the "unknown song" indicator
    }
    _LastSongDetectionTime = millis();
    _LastSongDisplayTime = millis() - 20000;  // forcem actualitzar el nom de la cançó asap
    if (!_SongDesconeguda) AddSongToHistory(_DetectedSongName, artworkUrl);
    SendDebugMessage(Utils::string_format("Detected SongName=[%s] artworkUrl=[%s]",
                                         _DetectedSongName.c_str(), artworkUrl).c_str());
  } else if (theTopic.find(TOPIC_THUMBNAIL) != std::string::npos) {
    SendDebugMessage(Utils::string_format("Detected Thumbnail. Datalen=%d bytes", dataLenght).c_str());
    DecodeThumbnail(pData, dataLenght);
    if (_TheDrawStyle != DRAW_STYLE::DRAW_THUMBNAIL) {
      _ThumbnailPrevStyle = _TheDrawStyle;         // remember the style when the thumbnail was received
      _ThumbnailPrevIntensity = _MAX_MILLIS;       // remember the intensity when the thumbnail was received
      ChangeDrawStyle(DRAW_STYLE::DRAW_THUMBNAIL, true);  // force the thumbnail to be drawn
    }
    _TimeThumbnailReceived = millis();
  } else if (theTopic.find(TOPIC_SHAZAM_MODE) != std::string::npos) {
    ApplyShazamModeSetting(std::atoi(theMsg.c_str()) != 0);
  } else if (theTopic.find(TOPIC_PIANO_MODE) != std::string::npos) {
    ApplyPianoModeSetting(std::atoi(theMsg.c_str()) != 0);
  } else if (theTopic.find(TOPIC_DEBUG_MODE) != std::string::npos) {
    auto newMode = std::atoi(theMsg.c_str()) != 0;
    if (newMode) {
      _DebugMode = newMode;
      SendDebugMessage("Debug Mode ON!!");
    } else {
      SendDebugMessage("Debug Mode OFF!!");
      _DebugMode = newMode;
    }
  } else if (theTopic.find(TOPIC_FADINGWAVE_MODE) != std::string::npos) {
    ApplyFadingWaveModeSetting(std::atoi(theMsg.c_str()) != 0);
  } else if (theTopic.find(TOPIC_TEMP_TERRASSA) != std::string::npos) {
    _LastTempTerrassa = std::strtof(theMsg.c_str(), nullptr);
    SendDebugMessage(Utils::string_format("Updated Temp Terrassa=%.2f°C", _LastTempTerrassa).c_str());
  } else if (theTopic.find(TOPIC_TEMP_INTERIOR) != std::string::npos) {
    _LastTempInterior = std::strtof(theMsg.c_str(), nullptr);
    SendDebugMessage(Utils::string_format("Updated Temp Interior=%.2f°C", _LastTempInterior).c_str());
  } else {
    log_w("Unknown topic [%s] with message [%s]", theTopic.c_str(), theMsg.c_str());
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
    SendDebugMessage("NO DATA");
  }
}

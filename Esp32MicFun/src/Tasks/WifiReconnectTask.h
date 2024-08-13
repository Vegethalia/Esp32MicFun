
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

//Experimental Task to show leds
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

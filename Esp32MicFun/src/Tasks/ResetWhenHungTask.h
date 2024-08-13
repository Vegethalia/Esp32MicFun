
void vTaskResetWhenHung(void* pvParameters)
{
    delay(60000);
    uint32_t lastFrameNum = _TheFrameNumber;

    int numChecks = 0;

    while (true) {
        delay(2000);
        numChecks++;
        if (numChecks > 30) {
            auto cc = _ThePrefs.getUShort(PREF_CURRENT_BY_MINUTES, 33);
            _ThePubSub.publish(TOPIC_DEBUG,
                Utils::string_format("** ResetCheck. CurrentFrame=%d OldFrame=%d. GroupByMinuts=%d **",
                    _TheFrameNumber, lastFrameNum, (int)cc)
                    .c_str(),
                true);
            numChecks = 0;
        }

        if (_OTA.Status() == OtaUpdater::OtaStatus::UPDATING) {
            continue;
        }

        if (_TheFrameNumber == lastFrameNum) {
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("HUNG DETECTED!! Reseting :(").c_str(), false);
            delay(2000);
            ESP.restart();
        }
        lastFrameNum = _TheFrameNumber;
    }
}

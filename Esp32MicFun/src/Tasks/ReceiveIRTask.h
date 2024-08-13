
/// @brief This task is responsible for receiving IR commands and processing them.
/// Updates some of the Global vars.
/// @param pvParameters 
void vTaskReceiveIR(void* pvParameters)
{
    // As this program is a special purpose capture/decoder, let us use a larger
    // than normal buffer so we can handle Air Conditioner remote codes.
    const uint16_t kCaptureBufferSize = 256;
    const uint8_t kTimeout = 50;
    // Set higher if you get lots of random short UNKNOWN messages when nothing
    // should be sending a message.
    // Set lower if you are sure your setup is working, but it doesn't see messages
    // from your device. (e.g. Other IR remotes work.)
    // NOTE: Set this value very high to effectively turn off UNKNOWN detection.
    const uint16_t kMinUnknownSize = 25; // 12;
    // How much percentage lee way do we give to incoming signals in order to match
    // it?
    // e.g. +/- 25% (default) to an expected value of 500 would mean matching a
    //      value between 375 & 625 inclusive.
    // Note: Default is 25(%). Going to a value >= 50(%) will cause some protocols
    //       to no longer match correctly. In normal situations you probably do not
    //       need to adjust this value. Typically that's when the library detects
    //       your remote's message some of the time, but not all of the time.
    const uint8_t kTolerancePercentage = 40; // kTolerance is normally 25%

    // Use turn on the save buffer feature for more complete capture coverage.
    IRrecv irrecv(PIN_RECEIVE_IR, kCaptureBufferSize, kTimeout, true);
    decode_results results; // Somewhere to store the results

    irrecv.setUnknownThreshold(kMinUnknownSize);
    irrecv.setTolerance(kTolerancePercentage); // Override the default tolerance.
    irrecv.enableIRIn(); // Start the receiver

    while (true) {
        if (irrecv.decode(&results)) {
            int32_t command = 0;
            if (results.repeat && results.command == 0) {
                command = _lastCommandIR;
            } else {
                command = results.command;
                _lastCommandIR = 0;
            }
            log_d("IR Decode:0x%2X IsRepeat=%d", results.command, results.repeat ? 1 : 0);
            _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("IR: Value=%d Command=0x%2X IsRepeat=%d", (uint32_t)results.value, results.command, results.repeat ? 1 : 0).c_str(), false);

            bool allowStyleChange = _TheDrawStyle != DRAW_STYLE::CALC_MODE;
            switch (command) {
            case IR_KEY_POWER:
                _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("IR RESTART").c_str(), false);
                delay(500);
                ESP.restart();
                break;
            case IR_KEY_STEP:
                if (allowStyleChange) {
                    _MAX_MILLIS = DEFAULT_MILLIS;
                    _TheDrawStyle = DRAW_STYLE::DEFAULT_STYLE;
                    FastLED.setBrightness(_MAX_MILLIS);

                    UpdatePref(Prefs::PR_STYLE);
                    UpdatePref(Prefs::PR_INTENSITY);

                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated intensity=%d. Style=%d", _MAX_MILLIS, (int)_TheDrawStyle).c_str(), true);
                }
                break;
            case IR_KEY_FADE2:
                SetNightMode(!_NightMode);
                UpdatePref(Prefs::PR_STYLE);
                UpdatePref(Prefs::PR_NIGHTMODE);

                _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("SetNightMode=%d. Style=%d", (int)_NightMode, (int)_TheDrawStyle).c_str(), true);
                break;
            case IR_KEY_INCBRIGHTNESS:
                _lastCommandIR = IR_KEY_INCBRIGHTNESS;
                if (_MAX_MILLIS < 20) {
                    _MAX_MILLIS++;
                } else if (_MAX_MILLIS < 50) {
                    _MAX_MILLIS += 5;
                } else if (_MAX_MILLIS < 100) {
                    _MAX_MILLIS += 10;
                } else if (_MAX_MILLIS < 225) {
                    _MAX_MILLIS += 25;
                } else {
                    _MAX_MILLIS = 255;
                }

                FastLED.setBrightness(_MAX_MILLIS);
                UpdatePref(Prefs::PR_INTENSITY);

                log_d("IR: INC BRIGHTNESS");
                _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated intensity=%d", _MAX_MILLIS).c_str(), true);
                break;
            case IR_KEY_DECBRIGHTNESS:
                _lastCommandIR = IR_KEY_DECBRIGHTNESS;
                if (_MAX_MILLIS > 225) {
                    _MAX_MILLIS = 225;
                } else if (_MAX_MILLIS > 100) {
                    _MAX_MILLIS -= 25;
                } else if (_MAX_MILLIS > 50) {
                    _MAX_MILLIS -= 10;
                } else if (_MAX_MILLIS > 20) {
                    _MAX_MILLIS -= 5;
                } else if (_MAX_MILLIS > 3) {
                    _MAX_MILLIS--;
                }

                FastLED.setBrightness(_MAX_MILLIS);
                UpdatePref(Prefs::PR_INTENSITY);
                log_d("IR: DEC BRIGHTNESS");

                _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated intensity=%dmAhs", _MAX_MILLIS).c_str(), false);
                break;
            case IR_KEY_EFFECT1:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::BARS_WITH_TOP;
                    UpdatePref(Prefs::PR_STYLE);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
                }
                break;
            case IR_KEY_EFFECT2:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::VERT_FIRE;
                    UpdatePref(Prefs::PR_STYLE);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
                }
                break;
            case IR_KEY_EFFECT3:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::HORIZ_FIRE;
                    UpdatePref(Prefs::PR_STYLE);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
                }
                break;
            case IR_KEY_EFFECT4:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::VISUAL_CURRENT;
                    _UpdateCurrentNow = true;
                    UpdatePref(Prefs::PR_STYLE);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
                }
                break;
            case IR_KEY_EFFECT5:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::MATRIX_FFT;
                    UpdatePref(Prefs::PR_STYLE);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
                }
                break;
            case IR_KEY_EFFECT6:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::DISCO_LIGTHS;
                    UpdatePref(Prefs::PR_STYLE);
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=%d", (int)_TheDrawStyle).c_str(), false);
                }
                break;
            case IR_KEY_JUMP1:
                _pianoMode = true;
                UpdatePref(Prefs::PR_PIANOMODE);
                _ThePubSub.publish(TOPIC_DEBUG, "PIANO MODE ON", false);
                break;
            case IR_KEY_JUMP2:
                _pianoMode = false;
                UpdatePref(Prefs::PR_PIANOMODE);
                _ThePubSub.publish(TOPIC_DEBUG, "PIANO MODE OFF", false);
                break;
            case IR_KEY_INCRED:
                _AgrupaConsumsPerMinuts = (uint16_t)max(min(_AgrupaConsumsPerMinuts + 1, (int)60), 1);
                UpdatePref(Prefs::PR_GROUPMINS);
                break;
            case IR_KEY_DECRED:
                _AgrupaConsumsPerMinuts = (uint16_t)max(min(_AgrupaConsumsPerMinuts - 1, (int)60), 1);
                UpdatePref(Prefs::PR_GROUPMINS);
                break;
            case IR_KEY_RED:
                _TheDesiredHue = HSVHue::HUE_RED;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_GREEN:
                _TheDesiredHue = HSVHue::HUE_GREEN;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_BLUE:
                _TheDesiredHue = HSVHue::HUE_BLUE;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_LIGHTRED:
                _TheDesiredHue = (HSVHue::HUE_RED + HSVHue::HUE_ORANGE) / 2;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_LIGHTGREEN:
                _TheDesiredHue = (HSVHue::HUE_GREEN + HSVHue::HUE_AQUA) / 2;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_LIGHTBLUE:
                _TheDesiredHue = HSVHue::HUE_AQUA;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_ORANGE:
                _TheDesiredHue = HSVHue::HUE_ORANGE;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_PURPLE:
                _TheDesiredHue = HSVHue::HUE_PURPLE;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_PINK:
                _TheDesiredHue = HSVHue::HUE_PINK;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_FUCSIA:
                _TheDesiredHue = (HSVHue::HUE_PURPLE + HSVHue::HUE_PINK) / 2;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_YELLOW:
                _TheDesiredHue = HSVHue::HUE_YELLOW;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_WHITE:
                _TheDesiredHue = -1;
                UpdatePref(Prefs::PR_CUSTOM_HUE);
                break;
            case IR_KEY_QUICK:
                if (allowStyleChange) {
                    _TheDrawStyle = DRAW_STYLE::CALC_MODE;
                    // UpdatePref(Prefs::PR_STYLE); //aquest mode no cal guardar-lo.
                    _ThePubSub.publish(TOPIC_DEBUG, Utils::string_format("Updated DrawStyle=CALC_MODE").c_str(), false);
                }
                break;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

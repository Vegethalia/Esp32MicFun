// Key Codes

// The following are the key codes for the common IR LedColor remote control.
#define IR_KEY_INCBRIGHTNESS 0x5C
#define IR_KEY_DECBRIGHTNESS 0x5D
#define IR_KEY_STEP 0x41
#define IR_KEY_POWER 0x40
#define IR_KEY_RED 0x58
#define IR_KEY_GREEN 0x59
#define IR_KEY_BLUE 0x45
#define IR_KEY_WHITE 0x44
#define IR_KEY_LIGHTRED 0x54
#define IR_KEY_LIGHTGREEN 0x55
#define IR_KEY_LIGHTBLUE 0x49
#define IR_KEY_PINK 0x48
#define IR_KEY_ORANGE 0x50
#define IR_KEY_AQUA 0x52
#define IR_KEY_PURPLE 0x4D
#define IR_KEY_YELLOW 0x18
#define IR_KEY_FUCSIA 0x1A
#define IR_KEY_LIGHTAQUA 0x1B
#define IR_KEY_INCRED 0x14
#define IR_KEY_DECRED 0x10
#define IR_KEY_INCGREEN 0x15
#define IR_KEY_DECGREEN 0x11
#define IR_KEY_INCBLUE 0x16
#define IR_KEY_DECBLUE 0x12
#define IR_KEY_QUICK 0x17
#define IR_KEY_SLOW 0x13
#define IR_KEY_EFFECT1 0x0C
#define IR_KEY_EFFECT2 0x0D
#define IR_KEY_EFFECT3 0x0E
#define IR_KEY_EFFECT4 0x08
#define IR_KEY_EFFECT5 0x09
#define IR_KEY_EFFECT6 0x0A
#define IR_KEY_FLASH 0x0B
#define IR_KEY_JUMP1 0x04
#define IR_KEY_JUMP2 0x05
#define IR_KEY_FADE3 0x06
#define IR_KEY_FADE7 0x07

// The following are the key codes for the DYON SCORPION IR remote control.
#define IR_KEY_DYON_POWER 0x18
#define IR_KEY_DYON_0 0x14
#define IR_KEY_DYON_1 0x1A
#define IR_KEY_DYON_2 0x11
#define IR_KEY_DYON_3 0x09
#define IR_KEY_DYON_4 0x1B
#define IR_KEY_DYON_5 0x12
#define IR_KEY_DYON_6 0x0A
#define IR_KEY_DYON_7 0x1C
#define IR_KEY_DYON_8 0x13
#define IR_KEY_DYON_9 0x0B
#define IR_KEY_DYON_RED 0x19
#define IR_KEY_DYON_GREEN 0x08
#define IR_KEY_DYON_YELLOW 0x03
#define IR_KEY_DYON_BLUE 0x1D
#define IR_KEY_DYON_OK 0x07
#define IR_KEY_DYON_UP 0x0D
#define IR_KEY_DYON_RIGHT 0x16
#define IR_KEY_DYON_DOWN 0x1E
#define IR_KEY_DYON_LEFT 0x17
#define IR_KEY_DYON_PLAY 0x45
#define IR_KEY_DYON_RECORD 0x41
#define IR_KEY_DYON_PAUSE 0x4C
#define IR_KEY_DYON_STOP 0x49
#define IR_KEY_DYON_INC 0x54
#define IR_KEY_DYON_DEC 0x58
#define IR_KEY_DYON_REFRESH 0x55
#define IR_KEY_DYON_ZOOM 0x59
#define IR_KEY_DYON_DISP 0x60

// Global Vars
int32_t _lastCommandIR = -1;

// Advanced declarations

/// @brief Processes the IR commands for the CalcMode. Only used when in CalcMode visualization style.
/// @param command
void ProcessIRCommand4CalcMode(uint32_t command);

/// @brief This task is responsible for receiving IR commands and processing them.
/// Updates some of the Global vars.
/// @param pvParameters
void vTaskReceiveIR(void* pvParameters) {
  // As this program is a special purpose capture/decoder, let us use a larger
  // than normal buffer so we can handle Air Conditioner remote codes.
  const uint16_t kCaptureBufferSize = 256;
  const uint8_t kTimeout = 50;
  // Set higher if you get lots of random short UNKNOWN messages when nothing
  // should be sending a message.
  // Set lower if you are sure your setup is working, but it doesn't see messages
  // from your device. (e.g. Other IR remotes work.)
  // NOTE: Set this value very high to effectively turn off UNKNOWN detection.
  const uint16_t kMinUnknownSize = 25;  // 12;
  // How much percentage lee way do we give to incoming signals in order to match
  // it?
  // e.g. +/- 25% (default) to an expected value of 500 would mean matching a
  //      value between 375 & 625 inclusive.
  // Note: Default is 25(%). Going to a value >= 50(%) will cause some protocols
  //       to no longer match correctly. In normal situations you probably do not
  //       need to adjust this value. Typically that's when the library detects
  //       your remote's message some of the time, but not all of the time.
  const uint8_t kTolerancePercentage = 40;  // kTolerance is normally 25%

  // Use turn on the save buffer feature for more complete capture coverage.
  IRrecv irrecv(PIN_RECEIVE_IR, kCaptureBufferSize, kTimeout, true);
  decode_results results;  // Somewhere to store the results

  irrecv.setUnknownThreshold(kMinUnknownSize);
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();                        // Start the receiver

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
      SendDebugMessage(Utils::string_format("IR: Value=%d Command=0x%2X IsRepeat=%d", (uint32_t)results.value, results.command, results.repeat ? 1 : 0).c_str());

      if (_TheDrawStyle == DRAW_STYLE::CALC_MODE) {
        ProcessIRCommand4CalcMode(command);
      }
      bool allowStyleChange = _TheDrawStyle != DRAW_STYLE::CALC_MODE;

      switch (command) {
        case IR_KEY_POWER:
          SendDebugMessage(Utils::string_format("IR RESTART").c_str());
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

            SendDebugMessage(Utils::string_format("Updated intensity=%d. Style=%d", _MAX_MILLIS, (int)_TheDrawStyle).c_str());
          }
          break;
        case IR_KEY_FADE3:
          if (_ShazamSongs) {
            _ShazamSongs = false;
            SendDebugMessage("Shazam Mode OFF!!");
          } else {
            _ShazamSongs = true;
            SendDebugMessage("Shazam Mode ON!!");
            _ShazamActivationTime = millis();
          }
          break;
        case IR_KEY_FLASH: //activem Shazam mode i enviem missatge al servei per a que es reconegui l'audio ASAP
          SendDebugMessage("Shazam Mode ON (ASAP)!!");
          _ShazamSongs = true;
          _ShazamActivationTime = millis();
          _DisplayAsapIndicator = true; // show the ASAP indicator on the display
          _AsapDetectionTime = millis(); // set the ASAP detection so it is active
          _ThePubSub.publish(TOPIC_RECOGNIZE_ASAP, "1", false);
          break;
        case IR_KEY_FADE7:
          if (allowStyleChange) {
            SetNightMode(!_NightMode);
            UpdatePref(Prefs::PR_STYLE);
            UpdatePref(Prefs::PR_NIGHTMODE);

            SendDebugMessage(Utils::string_format("SetNightMode=%d. Style=%d", (int)_NightMode, (int)_TheDrawStyle).c_str());
          }
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
          SendDebugMessage(Utils::string_format("Updated intensity=%d", _MAX_MILLIS).c_str());
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

          SendDebugMessage(Utils::string_format("Updated intensity=%dmAhs", _MAX_MILLIS).c_str());
          break;
        case IR_KEY_EFFECT1:
          ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP);
          break;
        case IR_KEY_EFFECT2:
          ChangeDrawStyle(DRAW_STYLE::VERT_FIRE);
          break;
        case IR_KEY_EFFECT3:
          ChangeDrawStyle(DRAW_STYLE::HORIZ_FIRE);
          break;
        case IR_KEY_EFFECT4:
          if (ChangeDrawStyle(DRAW_STYLE::VISUAL_CURRENT)) {
            _UpdateCurrentNow = true;
          }
          break;
        case IR_KEY_EFFECT5:
          ChangeDrawStyle(DRAW_STYLE::MATRIX_FFT);
          break;
        case IR_KEY_EFFECT6:
          ChangeDrawStyle(DRAW_STYLE::DISCO_LIGTHS);
          break;
        case IR_KEY_JUMP1:
          _pianoMode = true;
          UpdatePref(Prefs::PR_PIANOMODE);
          SendDebugMessage("PIANO MODE ON");
          break;
        case IR_KEY_JUMP2:
          _pianoMode = false;
          UpdatePref(Prefs::PR_PIANOMODE);
          SendDebugMessage("PIANO MODE OFF");
          break;
        case IR_KEY_INCRED:
          if (allowStyleChange) {
            if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
              _AgrupaConsumsPerMinuts = (uint16_t)max(min(_AgrupaConsumsPerMinuts + 1, (int)60), 1);
              UpdatePref(Prefs::PR_GROUPMINS);
            } else if (_TheDrawStyle == DRAW_STYLE::BARS_WITH_TOP) {
              _TheLastKey = GEN_KEY_PRESS::KEY_INC1;
            }
          }
          break;
        case IR_KEY_DECRED:
          if (allowStyleChange) {
            if (_TheDrawStyle == DRAW_STYLE::VISUAL_CURRENT) {
              _AgrupaConsumsPerMinuts = (uint16_t)max(min(_AgrupaConsumsPerMinuts - 1, (int)60), 1);
              UpdatePref(Prefs::PR_GROUPMINS);
            } else if (_TheDrawStyle == DRAW_STYLE::BARS_WITH_TOP) {
              _TheLastKey = GEN_KEY_PRESS::KEY_DEC1;
            }
          }
          break;
        case IR_KEY_INCBLUE:
          if (allowStyleChange) {
            _TheLastKey = GEN_KEY_PRESS::KEY_INC2;
          }
          break;
        case IR_KEY_DECBLUE:
          if (allowStyleChange) {
            _TheLastKey = GEN_KEY_PRESS::KEY_DEC2;
          }
          break;
        case IR_KEY_INCGREEN:
          if (allowStyleChange && _WaveDrawEvery < 5) {
            _WaveDrawEvery++;
            SendDebugMessage(Utils::string_format("_WaveDrawEvery=%d", _WaveDrawEvery).c_str());
          }
          break;
        case IR_KEY_DECGREEN:
          if (allowStyleChange && _WaveDrawEvery > 1) {
            _WaveDrawEvery--;
            SendDebugMessage(Utils::string_format("_WaveDrawEvery=%d", _WaveDrawEvery).c_str());
          }
          break;
        case IR_KEY_RED:
          _TheDesiredHue = HSVHue::HUE_RED;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_GREEN:
          _TheDesiredHue = HSVHue::HUE_GREEN;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_BLUE:
          _TheDesiredHue = HSVHue::HUE_BLUE;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_LIGHTRED:
          _TheDesiredHue = (HSVHue::HUE_RED + HSVHue::HUE_ORANGE) / 2;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_LIGHTGREEN:
          _TheDesiredHue = (HSVHue::HUE_GREEN + HSVHue::HUE_AQUA) / 2;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_LIGHTBLUE:
          _TheDesiredHue = HSVHue::HUE_AQUA;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_ORANGE:
          _TheDesiredHue = HSVHue::HUE_ORANGE;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_PURPLE:
          _TheDesiredHue = HSVHue::HUE_PURPLE;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_PINK:
          _TheDesiredHue = HSVHue::HUE_PINK;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_FUCSIA:
          _TheDesiredHue = (HSVHue::HUE_PURPLE + HSVHue::HUE_PINK) / 2;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_YELLOW:
          _TheDesiredHue = HSVHue::HUE_YELLOW;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_WHITE:
          _TheDesiredHue = -1;
          UpdatePref(Prefs::PR_CUSTOM_HUE);
          _DesiredHueLastSet = millis();
          break;
        case IR_KEY_DYON_DISP:
        case IR_KEY_QUICK:
          if (ChangeDrawStyle(DRAW_STYLE::CALC_MODE)) {
            // UpdatePref(Prefs::PR_STYLE); //aquest mode no cal guardar-lo.
            SendDebugMessage(Utils::string_format("Updated DrawStyle=CALC_MODE").c_str());
          }
          break;
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void ProcessIRCommand4CalcMode(uint32_t command) {
  switch (command) {
    case IR_KEY_DYON_0:
      _TheLastKey = GEN_KEY_PRESS::KEY_0;
      break;
    case IR_KEY_DYON_1:
      _TheLastKey = GEN_KEY_PRESS::KEY_1;
      break;
    case IR_KEY_DYON_2:
      _TheLastKey = GEN_KEY_PRESS::KEY_2;
      break;
    case IR_KEY_DYON_3:
      _TheLastKey = GEN_KEY_PRESS::KEY_3;
      break;
    case IR_KEY_DYON_4:
      _TheLastKey = GEN_KEY_PRESS::KEY_4;
      break;
    case IR_KEY_DYON_5:
      _TheLastKey = GEN_KEY_PRESS::KEY_5;
      break;
    case IR_KEY_DYON_6:
      _TheLastKey = GEN_KEY_PRESS::KEY_6;
      break;
    case IR_KEY_DYON_7:
      _TheLastKey = GEN_KEY_PRESS::KEY_7;
      break;
    case IR_KEY_DYON_8:
      _TheLastKey = GEN_KEY_PRESS::KEY_8;
      break;
    case IR_KEY_DYON_9:
      _TheLastKey = GEN_KEY_PRESS::KEY_9;
      break;
    case IR_KEY_DYON_RED:
      _TheLastKey = GEN_KEY_PRESS::KEY_MINUS;
      break;
    case IR_KEY_DYON_GREEN:
      _TheLastKey = GEN_KEY_PRESS::KEY_PLUS;
      break;
    case IR_KEY_DYON_YELLOW:
      _TheLastKey = GEN_KEY_PRESS::KEY_PROD;
      break;
    case IR_KEY_DYON_BLUE:
      _TheLastKey = GEN_KEY_PRESS::KEY_DIV;
      break;
    case IR_KEY_DYON_OK:
      _TheLastKey = GEN_KEY_PRESS::KEY_ENTER;
      break;
    case IR_KEY_DYON_UP:
      _TheLastKey = GEN_KEY_PRESS::KEY_UP;
      break;
    case IR_KEY_DYON_DOWN:
      _TheLastKey = GEN_KEY_PRESS::KEY_DOWN;
      break;
    case IR_KEY_DYON_LEFT:
      _TheLastKey = GEN_KEY_PRESS::KEY_LEFT;
      break;
    // case IR_KEY_INCBLUE:
    case IR_KEY_DYON_RIGHT:
      _TheLastKey = GEN_KEY_PRESS::KEY_RIGHT;
      break;
    case IR_KEY_DYON_REFRESH:
      _TheLastKey = GEN_KEY_PRESS::KEY_REFRESH;
      break;
    case IR_KEY_DYON_POWER:
      ChangeDrawStyle(DRAW_STYLE::BARS_WITH_TOP, true);
      break;
  }
}

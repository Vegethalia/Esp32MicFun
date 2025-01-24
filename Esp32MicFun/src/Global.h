#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
// #define PIN_BASS_LED 33
#define PIN_DATA_LEDS1 16
#define PIN_DATA_LEDS2 4
#define PIN_DATA_LEDS3 2
#define PIN_DATA_LEDS4 15
#define PIN_RECEIVE_IR GPIO_NUM_34
#define PIN_AUDIO_IN ADC1_CHANNEL_4 /*!< ADC1 channel 4 is GPIO32 */
#define BUS_SPEED 800000

#define PIN_MIC_I2S_WS 5
#define PIN_MIC_I2S_SD 33
#define PIN_MIC_I2S_SCK 25

#define WITH_VISUALCURRENT true
#define WITH_MEMS_MIC true

#define INIT_SCREEN false // true
#define USE_SCREEN false
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DEFAULT_VREF 1100 // ajusta el valor de referència per a la lectura per ADC
#define INPUT_0_VALUE 1240 // input is biased towards 1.5V
#define VOLTATGE_DRAW_RANGE 400 // total range is this value*2. in millivolts. 400 imply a visible range from [INPUT_0_VALUE-400]....[INPUT_0_VALUE+400]
uint32_t MAX_FFT_MAGNITUDE = 2500000; // 75000 // a magnitude greater than this value will be considered Max Power
#define MIN_FFT_DB -60 // a magnitude under this value will be considered 0 (noise)
#define MAX_FFT_DB 0 // a magnitude greater than this value will be considered Max Power

#define FFT_SIZE 2048
#define HZ_PER_BIN 6
#define TARGET_SAMPLE_RATE (FFT_SIZE * HZ_PER_BIN) // 10240 // 20480 // 11025 // 8192 //11025 //9984//9728//10752 //10496 //10240 //9216
#define OVERSAMPLING 4 // we will oversample by this amount
#define SAMPLE_RATE (TARGET_SAMPLE_RATE * OVERSAMPLING) // we will oversample by 2. We can only draw up to 5kpixels per second
#define BYTES_X_SAMPLE 4

#define AUDIO_DATA_OUT (SCREEN_WIDTH * 2)
// #define VISUALIZATION FftPower::AUTO34

#define MASK_12BIT 0x0fff

#define RETRY_WIFI_EVERY_SECS 10
#define UPDATE_CONSUM_ELECTRICITAT_CADA_MS (120 * 1000)

U8G2_SH1106_128X64_NONAME_F_HW_I2C _u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 255, PIN_I2C_SCL, PIN_I2C_SDA);

// #define BARS_RESOLUTION 8 // 8=32 4=64 2=128
//  #define BARS_RESOLUTION 4 // 8=32 4=64 2=128

#define DEFAULT_MILLIS 40
#define MAX_MILLIS 2000
#define NIGHT_MILLIS 4
uint16_t _MAX_MILLIS = DEFAULT_MILLIS;

// #define BAR_HEIGHT (PANEL_HEIGHT_16 - 1) // we have this amount of "vertical leds" per bar. 0 based.
// #define THE_PANEL_WIDTH PANEL_WIDTH_33
// #define NUM_LEDS (THE_PANEL_WIDTH * PANEL_HEIGHT_16) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32
#define THE_PANEL_HEIGHT PANEL_HEIGHT_32 // PANEL_HEIGHT_8
#define THE_PANEL_WIDTH PANEL_WIDTH_64 // PANEL_WIDTH_33 // PANEL_WIDTH_64
#define BAR_HEIGHT (THE_PANEL_HEIGHT - 1) // we have this amount of "vertical leds" per bar. 0 based.
#define NUM_LEDS (THE_PANEL_WIDTH * THE_PANEL_HEIGHT) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32

#define MAX_FADING_WAVES 1 // 2 // number of waves maintained "alive". Every frame will paint the previous NUM_FADING_WAVES (darker) before painting the latest in front of the rest

CRGBArray<NUM_LEDS> _TheLeds;
// PanelMapping33x16 _TheMapping;
// PanelMapping64x8 _TheMapping;
// PanelMapping64x16 _TheMapping;
// PanelMapping64x24 _TheMapping;
PanelMapping64x32 _TheMapping;
PowerBarsPanel<NUM_LEDS, THE_PANEL_WIDTH, THE_PANEL_HEIGHT> _ThePanel;

OtaUpdater _OTA;
PubSubClient _ThePubSub;
WiFiClient _TheWifi;

uint32_t _InitTime = 0;
uint16_t _numFrames = 0;
uint32_t _TheFrameNumber = 0;
uint32_t _LastMqttReconnect = 0;
float _fps = 0.0f;
bool _Connected2Wifi = false;
bool _WithClock = true;
bool _pianoMode = false; // true;
bool _DemoMode = true; // when the device starts, execute a sequence of "demo" figures
bool _NightMode = false; // true if night mode is ON --> dimm colors
uint32_t _DemoModeFrame = 0; // when  _DemoMode is true, this counts the number of frames and allows to move from one state to the next
byte _WaveDrawEvery = 1; // to jump some pixels when drawing the wave

uint32_t _LastCheck4Wifi = 0;

esp_adc_cal_characteristics_t* _adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));

// MQTT related
#define MQTT_BROKER "192.168.1.140"
#define MQTT_PORT 1888
#define MQTT_RECONNECT_IGNORE_MSG_MS 30000

#define TOPIC_INTENSITY "caseta/spectrometre/intensity"
#define TOPIC_DELAYFRAME "caseta/spectrometre/delay"
#define TOPIC_DEBUG "caseta/spectrometre/debug"
#define TOPIC_FPS "caseta/spectrometre/fps"
#define TOPIC_STYLE "caseta/spectrometre/style"
#define TOPIC_WITHWAVE "caseta/spectrometre/wave"
#define TOPIC_BASEHUE "caseta/spectrometre/basehue"
#define TOPIC_RESET "caseta/spectrometre/reset"
#define TOPIC_NIGHTMODE "caseta/spectrometre/nightmode"
#define TOPIC_GROUPMINUTS "caseta/spectrometre/groupbyminuts"
#define TOPIC_LASTIP "caseta/spectrometre/lastip"

#define TOPIC_FREEHEAP "caseta/spectrometre/freeheap"
#define TOPIC_BIGGESTFREEBLOCK "caseta/spectrometre/largestfreeblock"
#define TOPIC_HIWATER_READER "caseta/spectrometre/hiwreader"
#define TOPIC_HIWATER_DRAWER "caseta/spectrometre/hiwdrawer"
#define TOPIC_HIWATER_WIFI "caseta/spectrometre/hiwWIFI"
#define TOPIC_HIWATER_IRDECO "caseta/spectrometre/hiwIRdeco"
#define TOPIC_CPU_WIFI "caseta/spectrometre/cpuwifi"
#define TOPIC_CPU_READER "caseta/spectrometre/cpureader"
#define TOPIC_CPU_DRAWER "caseta/spectrometre/cpudrawer"
#define TOPIC_CURRENT_WH "caseta/spectrometre/currentwh"
#define TOPIC_LIVEAUDIO "caseta/spectrometre/liveaudio"

// Preferences related
Preferences _ThePrefs;

enum Prefs {
    PR_INTENSITY, // uses _MAX_MILLIS
    PR_STYLE, // uses _TheDrawStyle
    PR_GROUPMINS, // uses  _AgrupaConsumsPerMinuts
    PR_NIGHTMODE, // uses _NightMode
    PR_PIANOMODE, // uses _pianoMode
    PR_CUSTOM_HUE // uses _TheDesiredHue
};

#define PREF_STYLE "Style"
#define PREF_INTENSITY "Intensity"
#define PREF_PIANOMODE "PianoMode"
#define PREF_NIGHTMODE "NightMode"
#define PREF_CURRENT_BY_MINUTES "CurrentByMins"
#define PREF_CUSTOM_HUE "CustomHue"

// Mems mic inmp441
// gnd --> gnd
// vdd --> 3.3v
// sd  --> gpio33
// sck --> gpio25
// ws  --> gpio5
// l/r --> gnd (left)

//------------
// IR Related
//------------
enum GEN_KEY_PRESS {
    KEY_NONE,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_PROD,
    KEY_DIV,
    KEY_MINUS,
    KEY_PLUS,
    KEY_ENTER,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_REFRESH,
    KEY_INC1,
    KEY_DEC1,
    KEY_INC2,
    KEY_DEC2,
};

GEN_KEY_PRESS _TheLastKey = GEN_KEY_PRESS::KEY_NONE;

//------------
// Task Related
//------------
TaskHandle_t _readerTaskHandle;
TaskHandle_t _drawTaskHandle;
// TaskHandle_t _drawTaskShowLeds;
// TaskHandle_t _showLedsTaskHandle;
TaskHandle_t _wifiReconnectTaskHandle;
// TaskHandle_t _refrescarConsumTaskHandle;
TaskHandle_t _receiveIRTaskHandle;
TaskHandle_t _resetWhenHungTaskHandle;

QueueHandle_t _adc_i2s_event_queue, _xQueSendAudio2Drawer, _xQueSendFft2Led;
uint8_t _adc_i2s_event_queue_size = 1;

volatile bool _Drawing = false;
struct TaskParams {
    int16_t data1[AUDIO_DATA_OUT * 4];
    // uint16_t data2[AUDIO_DATA_OUT];
    int8_t fftMag[FFT_SIZE / 4]; // fftMag[AUDIO_DATA_OUT / 2];
    // uint16_t dataOrig[AUDIO_DATA_OUT * OVERSAMPLING];
    // uint8_t lastBuffSet; // 0=none, 1=data1, 2=data2

    TaskParams()
    {
        //        lastBuffSet = 0;
    }
};
TaskParams _TaskParams;

struct MsgAudio2Draw {
    int16_t* pAudio;
    uint32_t audioLenInSamples;
    int8_t* pFftMag;
    uint16_t sizeFftMagVector;
    uint16_t max_freq;
    uint8_t* pDBs; // magnitude in dBs on each bin of the resulting FFT (pFftMag) 
};

enum DRAW_STYLE {
    BARS_WITH_TOP = 1,
    VERT_FIRE = 2,
    HORIZ_FIRE = 3,
    VISUAL_CURRENT = 4,
    MATRIX_FFT = 5,
    DISCO_LIGTHS = 6,
    CALC_MODE = 7,

    MAX_STYLE = CALC_MODE,
    DEFAULT_STYLE = MATRIX_FFT
};
DRAW_STYLE _TheDrawStyle = DRAW_STYLE::DEFAULT_STYLE;
int16_t _TheDesiredHue = -1; // el custom color a aplicar. -1 -> color per defecte.
uint8_t _1stBarValue = 128;

// Calculator Mode
#define CALC_MODE_SHOWTITLE_MS 6000
int32_t _StartedCalcMode = -1;
bool _AllLinesUsed = false; // true if all lines from top to bottom of panel are used

// consum electricitat
#define DEFAULT_CONSUM_PER_MINUTS 2 // per defecte cada bar seran 2 minuts
#define DEFAULT_MAX_WH 3500 // per defecte es mostrara un rang vertical de 3.5 kWh
uint16_t _AgrupaConsumsPerMinuts = DEFAULT_CONSUM_PER_MINUTS;
uint16_t _MaxWhToShow = DEFAULT_MAX_WH;
uint16_t _MapMaxWhToPixels = 3000; // THE_PANEL_HEIGHT - 9 - 1; // 9 pixels pel clock, 1 per l'eix horitzontal
struct LecturaConsum {
    time_t horaConsum;
    uint16_t valorEnLeds; // el valor de KWh mapejat a LEDS verticals a pintar
};

time_t _lastCurrentTime = 0; // hora de l'últim valor de corrent publicat
LecturaConsum* _pLectures = nullptr;
bool _UpdateCurrentNow = false; // true if current must be updated ASAP

uint8_t
GetMapMaxPixels()
{
    uint8_t ret = 20;
    if (_MapMaxWhToPixels == 1000 || _MapMaxWhToPixels == 2000 || _MapMaxWhToPixels == 2500 || _MapMaxWhToPixels == 4000 || _MapMaxWhToPixels == 5000) {
        ret = 20; // 20/10/8/5/4 leds per kWh
    } else if (_MapMaxWhToPixels == 1500 || _MapMaxWhToPixels == 3000 || _MapMaxWhToPixels == 3500) {
        ret = 21; // 14/7/6 leds per kWh
    } else if (_MapMaxWhToPixels == 4500 || _MapMaxWhToPixels == 6000) {
        ret = 18; // 4/3 leds per kWh
    } else if (_MapMaxWhToPixels == 5500) {
        ret = 22; // 4 leds per kWh
    }
    return ret;
}

uint8_t GetPixelsPerKwh(uint8_t maxPixels)
{
    uint16_t aux = maxPixels * 1000;
    return (uint8_t)(aux / _MapMaxWhToPixels);
}

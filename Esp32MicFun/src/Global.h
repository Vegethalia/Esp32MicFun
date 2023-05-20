#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_BASS_LED 33
#define PIN_DATA_LEDS1 16
#define PIN_DATA_LEDS2 4
#define PIN_DATA_LEDS3 2
#define PIN_DATA_LEDS4 15
#define BUS_SPEED 800000

#define INIT_SCREEN true
#define USE_SCREEN false
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DEFAULT_VREF 1100
#define INPUT_0_VALUE 1225 // input is biased towards 1.5V
#define VOLTATGE_DRAW_RANGE 800 // total range is this value*2. in millivolts. 400 imply a visible range from [INPUT_0_VALUE-400]....[INPUT_0_VALUE+400]
#define MAX_FFT_MAGNITUDE 100000 // 75000 // a magnitude greater than this value will be considered Max Power
#define MIN_FFT_DB -55 // a magnitude under this value will be considered 0 (noise)
#define MAX_FFT_DB 5 // a magnitude greater than this value will be considered Max Power

#define FFT_SIZE 2048
#define TARGET_SAMPLE_RATE (FFT_SIZE * 6) // 10240 // 20480 // 11025 // 8192 //11025 //9984//9728//10752 //10496 //10240 //9216
#define OVERSAMPLING 2 // we will oversample by this amount
#define SAMPLE_RATE (TARGET_SAMPLE_RATE * OVERSAMPLING) // we will oversample by 2. We can only draw up to 5kpixels per second

#define AUDIO_DATA_OUT (SCREEN_WIDTH * 2)
// #define VISUALIZATION FftPower::AUTO34

#define MASK_12BIT 0x0fff

#define RETRY_WIFI_EVERY_SECS 10

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
float _fps = 0.0f;
bool _Connected2Wifi = false;
bool _WithClock = true;
bool _pianoMode = true;
bool _DemoMode = true; // when the device starts, execute a sequence of "demo" figures
uint32_t _DemoModeFrame = 0; // when  _DemoMode is true, this counts the number of frames and allows to move from one state to the next

uint32_t _LastCheck4Wifi = 0;

esp_adc_cal_characteristics_t* _adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));

// MQTT related
#define MQTT_BROKER "192.168.1.140"
#define MQTT_PORT 1888

#define TOPIC_INTENSITY "caseta/spectrometre/intensity"
#define TOPIC_DELAYFRAME "caseta/spectrometre/delay"
#define TOPIC_DEBUG "caseta/spectrometre/debug"
#define TOPIC_FPS "caseta/spectrometre/fps"
#define TOPIC_STYLE "caseta/spectrometre/style"
#define TOPIC_WITHWAVE "caseta/spectrometre/wave"
#define TOPIC_BASEHUE "caseta/spectrometre/basehue"
#define TOPIC_RESET "caseta/spectrometre/reset"
#define TOPIC_NIGHTMODE "caseta/spectrometre/nightmode"

//------------
// Task Related
//------------
TaskHandle_t _readerTaskHandle;
TaskHandle_t _drawTaskHandle;
TaskHandle_t _drawTaskShowLeds;
TaskHandle_t _showLedsTaskHandle;
TaskHandle_t _wifiReconnectTaskHandle;

QueueHandle_t _adc_i2s_event_queue, _xQueSendAudio2Drawer, _xQueSendFft2Led;
uint8_t _adc_i2s_event_queue_size = 1;

bool _Drawing = false;
struct TaskParams {
    uint16_t data1[AUDIO_DATA_OUT * 4];
    // uint16_t data2[AUDIO_DATA_OUT];
    int32_t fftMag[AUDIO_DATA_OUT / 2];
    // uint16_t dataOrig[AUDIO_DATA_OUT * OVERSAMPLING];
    // uint8_t lastBuffSet; // 0=none, 1=data1, 2=data2

    TaskParams()
    {
        //        lastBuffSet = 0;
    }
};
TaskParams _TaskParams;

struct MsgAudio2Draw {
    uint16_t* pAudio;
    int32_t* pFftMag;
    uint16_t max_freq;
};

enum DRAW_STYLE {
    BARS_WITH_TOP = 1,
    VERT_FIRE = 2,
    HORIZ_FIRE = 3,

    MAX_STYLE = HORIZ_FIRE
};

DRAW_STYLE _TheDrawStyle = DRAW_STYLE::VERT_FIRE;

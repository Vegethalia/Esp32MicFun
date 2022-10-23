#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_BASS_LED 33
#define PIN_DATA_LEDS 16
#define BUS_SPEED 800000

#define USE_SCREEN false
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DEFAULT_VREF 1100
#define INPUT_0_VALUE 1225 // input is biased towards 1.5V
#define VOLTATGE_DRAW_RANGE 900 // total range is this value*2. in millivolts. 400 imply a visible range from [INPUT_0_VALUE-400]....[INPUT_0_VALUE+400]
#define MAX_FFT_MAGNITUDE 75000 // a magnitude greater than this value will be considered Max Power
#define MIN_FFT_DB -45 // a magnitude under this value will be considered 0 (noise)
#define MAX_FFT_DB 0 // a magnitude greater than this value will be considered Max Power

#define TARGET_SAMPLE_RATE 16000 // 11025 // 8192 //11025 //9984//9728//10752 //10496 //10240 //9216
#define OVERSAMPLING 2 // we will oversample by this amount
#define SAMPLE_RATE (TARGET_SAMPLE_RATE * OVERSAMPLING) // we will oversample by 2. We can only draw up to 5kpixels per second

#define AUDIO_DATA_OUT (SCREEN_WIDTH * 2)
#define VISUALIZATION FftPower::AUTO34

#define MASK_12BIT 0x0fff

#define RETRY_WIFI_EVERY_SECS 60

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 255, PIN_I2C_SCL, PIN_I2C_SDA);

#define BARS_RESOLUTION 8 // 8=32 4=64 2=128

#define DEFAULT_MILLIS 600
#define MAX_MILLIS 2000
uint16_t _MAX_MILLIS = DEFAULT_MILLIS;

#define BAR_HEIGHT (PANEL_HEIGHT_16 - 1) // we have this amount of "vertical leds" per bar. 0 based.
#define NUM_LEDS (PANEL_WIDTH_33 * PANEL_HEIGHT_16) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32
CRGBArray<NUM_LEDS> _TheLeds;
PanelMapping33x16 _TheMapping;
PowerBarsPanel<NUM_LEDS, PANEL_WIDTH_33, PANEL_HEIGHT_16> _ThePanel;

OtaUpdater _OTA;
PubSubClient _ThePubSub;
WiFiClient _TheWifi;
WiFiUDP _TheWifi4UDP;
NTPClient _TheNTPClient(_TheWifi4UDP);

// uint8_t _ScreenBrightness = 0;
uint32_t _InitTime = 0;
uint16_t _numFrames = 0;
float _fps = 0.0f;
// bool _BassOn = false;
bool _Connected2Wifi = false;

uint32_t _LastCheck4Wifi = 0;

esp_adc_cal_characteristics_t* _adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));

// MQTT related
#define MQTT_BROKER "192.168.1.140"
#define MQTT_PORT 1888

#define TOPIC_INTENSITY "caseta/spectrometre/intensity"
#define TOPIC_DELAYFRAME "caseta/spectrometre/delay"
#define TOPIC_DEBUG "caseta/spectrometre/debug"

//------------
// Task Related
//------------
TaskHandle_t _readerTaskHandle;
TaskHandle_t _drawTaskHandle;
TaskHandle_t _showLedsTaskHandle;
TaskHandle_t _wifiReconnectTaskHandle;

QueueHandle_t _adc_i2s_event_queue, _xQueSendAudio2Drawer, _xQueSendFft2Led;
uint8_t _adc_i2s_event_queue_size = 1;

struct TaskParams {
    uint16_t data1[AUDIO_DATA_OUT];
    uint16_t data2[AUDIO_DATA_OUT];
    int32_t fftMag[AUDIO_DATA_OUT / 2];
    uint16_t dataOrig[AUDIO_DATA_OUT * OVERSAMPLING];
    uint8_t lastBuffSet; // 0=none, 1=data1, 2=data2

    TaskParams()
    {
        lastBuffSet = 0;
    }
};
TaskParams _TaskParams;

struct MsgAudio2Draw {
    uint16_t* pAudio;
    int32_t* pFftMag;
    uint16_t max_freq;
};

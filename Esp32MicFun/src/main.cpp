#include <Arduino.h>
#include <U8g2lib.h>
#include <memory>
#include <FastLED.h>
#include "SharedUtils\Utils.h"
#include "FftPower.h"
#include "PowerBarsPanel.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
//#include <components/freertos/FreeRTOS-Kernel/incl                 ude/freertos/queue.h>

#define PIN_I2C_SDA   21
#define PIN_I2C_SCL   22
#define PIN_BASS_LED  33
#define PIN_DATA_LEDS 16
#define BUS_SPEED   800000

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DEFAULT_VREF       1100
#define INPUT_0_VALUE      1225  //input is biased towards 1.5V
#define VOLATGE_DRAW_RANGE 900   //total range is this value*2. in millivolts. 400 imply a visible range from [INPUT_0_VALUE-400]....[INPUT_0_VALUE+400]
#define MAX_FFT_MAGNITUDE  75000  //a magnitude greater than this value will be considered Max Power
#define MIN_FFT_DB         -45    //a magnitude under this value will be considered 0 (noise)
#define MAX_FFT_DB         0     //a magnitude greater than this value will be considered Max Power

#define TARGET_SAMPLE_RATE  11025 // 8192 //11025 //9984//9728//10752 //10496 //10240 //9216
#define OVERSAMPLING       4     //we will oversample by this amount
#define SAMPLE_RATE        (TARGET_SAMPLE_RATE*OVERSAMPLING) //we will oversample by 2. We can only draw up to 5kpixels per second

#define AUDIO_DATA_OUT     (SCREEN_WIDTH*2)
#define VISUALIZATION      FftPower::AUTO34

#define MASK_12BIT 0x0fff

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 255, PIN_I2C_SCL, PIN_I2C_SDA);

#define BARS_RESOLUTION 8 //8=32 4=64 2=128

#define MAX_MILLIS   1500
#define BAR_HEIGHT   (PANEL_HEIGHT_13-1)    //we havve this amount of "vertical leds" per bar. 0 based.
#define NUM_LEDS     (PANEL_WIDTH_33*PANEL_HEIGHT_13) //(VISUALIZATION==FftPower::AUTO34?33:(AUDIO_DATA_OUT/BARS_RESOLUTION)) //198//32
CRGBArray<NUM_LEDS>  _TheLeds;
PanelMapping33x13 _TheMapping;
PowerBarsPanel<NUM_LEDS, PANEL_WIDTH_33, PANEL_HEIGHT_13> _ThePanel;

uint8_t _ScreenBrightness=0;
uint32_t _InitTime=0;
uint8_t  _numFrames = 0;
float _fps=0.0f;
bool _BassOn=false;

int16_t _PosX=13;
int16_t _PosY=37;
uint8_t _ballRadius = 2;
int8_t _incX=7;
int8_t _incY=3;
int8_t _incRadius=1;
bool _dreta = true;
bool _abaix = true;

esp_adc_cal_characteristics_t* _adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

TaskHandle_t _readerTaskHandle;
TaskHandle_t _drawTaskHandle;
TaskHandle_t _showLedsTaskHandle;

QueueHandle_t _adc_i2s_event_queue, _xQueSendAudio2Drawer, _xQueSendFft2Led;
uint8_t _adc_i2s_event_queue_size = 1;


struct TaskParams {
	uint16_t data1[AUDIO_DATA_OUT];
	uint16_t data2[AUDIO_DATA_OUT];
	int32_t fftMag[AUDIO_DATA_OUT / 2];
	uint16_t dataOrig[AUDIO_DATA_OUT * OVERSAMPLING];
	uint8_t lastBuffSet; //0=none, 1=data1, 2=data2

	TaskParams() {
		lastBuffSet=0;
	}
};
TaskParams _TaskParams;

struct MsgAudio2Draw {
	uint16_t* pAudio;
	int32_t* pFftMag;
	uint16_t max_freq;
};

void DrawLedBars(MsgAudio2Draw& mad)
{
	uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
	uint8_t maxBassValue = 0;
	int16_t value = 0;

	if(VISUALIZATION == FftPower::AUTO34) {
		maxIndex = 34;

	}
	//_TheLeds.fill_solid(CRGB(1,1,1));
	FastLED.clear();
	assert(BAR_HEIGHT>1);
	uint8_t minBoostBin = (uint8_t)(maxIndex * 0.33); //the first 13 bars in 33 width panel
	constexpr float maxTrebleBoost = 30.0;
	constexpr float minBassBoost = 1.0;
	float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)maxIndex);

	for(uint16_t i = 1; i < maxIndex; i++) {
		if(i > minBoostBin) { //boost hi frequencies (to make them more visible)
			auto boost = 1.0f + (i * freqBoost);
			value = (int)(value * boost);
		}

		value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
		value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, (BAR_HEIGHT*10)+9); //fins a 89

		 _ThePanel.DrawBar(i-1, value);
	}

	FastLED.show();
}

void DrawLedsSingleLine(MsgAudio2Draw& mad)
{
	uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
	uint8_t maxBassValue = 0;
	int16_t value = 0;

	if(VISUALIZATION==FftPower::AUTO34) {
		maxIndex=34;
	}
//log_d("maxIndex=%d", maxIndex);
	for(uint16_t i = 1; i < maxIndex; i++) {
		value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
		value = map(value, MIN_FFT_DB, MAX_FFT_DB, 1, 128);

		if(value > 100) {
			_TheLeds[NUM_LEDS - i] = CHSV(HSVHue::HUE_AQUA, 255, 128);
		}
		// else if(value<=2) {
		// 	_TheLeds[NUM_LEDS - i - 1] = CRGB(1, 1, 1);//CHSV(HSVHue::HUE_PURPLE, 255, value);
		// }
		else {
			_TheLeds[NUM_LEDS - i] = CHSV(HSVHue::HUE_BLUE, 255, value);//CRGB(value, value, value);//CHSV(HSVHue::HUE_PURPLE, 255, value);
		}
		// if(i<4 && value>maxBassValue) { //BASS bins
		// 	maxBassValue = value;
		// }
	}
	FastLED.show();
}

// Task to read samples.
void vTaskReader(void* pvParameters)
{
	size_t buffSize = sizeof(_TaskParams.data1);
	size_t buffSizeOrig = sizeof(_TaskParams.dataOrig);
	size_t bytesRead=0;
	log_d("Estic a la ReaderTask. BuffSize=[%d]", buffSize);
	i2s_event_t adc_i2s_evt;
	uint32_t recInit=millis();
	uint32_t totalSamples=0;
	uint16_t numCalls=0;
	int32_t superMaxMag=-10000;
	MsgAudio2Draw mad;
	FftPower theFFT(AUDIO_DATA_OUT);
	float *pInputFft=theFFT.GetInputBuffer();

	// Create the FFT config structure
	//fft_config_t* real_fft_plan = fft_init(AUDIO_DATA_OUT, FFT_REAL, FFT_FORWARD, NULL, NULL);
	float freqs_x_bin = (float)(TARGET_SAMPLE_RATE) / (float)AUDIO_DATA_OUT;
	log_d("Freqs_x_Bin=%3.2f", freqs_x_bin);

	for(;; ) {
		if(xQueueReceive(_adc_i2s_event_queue, (void*)&adc_i2s_evt, (portTickType)portMAX_DELAY)) {
			if(adc_i2s_evt.type == I2S_EVENT_RX_DONE) {
				uint16_t* pDest = _TaskParams.lastBuffSet == 2 ? _TaskParams.data1 : _TaskParams.data2;

				_TaskParams.lastBuffSet = _TaskParams.lastBuffSet == 2 ? 1 : 2;
				auto err = i2s_read(I2S_NUM_0, (void*)_TaskParams.dataOrig, buffSizeOrig, &bytesRead, portMAX_DELAY);
				if(err != ESP_OK) {
					log_d("is_read error! [%d]", err);
				}
				else {
					if(bytesRead != buffSizeOrig) {
						log_d("bytesRead=%d", bytesRead);
					}
					uint16_t samplesRead = bytesRead / sizeof(uint16_t);
					totalSamples += samplesRead;
					numCalls++;
					//_TaskParams.buffNumber++;
					//now we convert the values to mv from 1V to 3V
					//int k=0;
					//log_d("BytesRead=%d buzzSizeOrig=%d SamplesRead=%d TotalSamples=%d", bytesRead, buffSizeOrig, samplesRead, totalSamples);
					for(int i = 0, k=0; i < samplesRead; i += OVERSAMPLING, k++) {
						pDest[k] = 0;
						for(uint8_t ov=0; ov<OVERSAMPLING; ov++) {
							pDest[k] += (uint16_t)(esp_adc_cal_raw_to_voltage(_TaskParams.dataOrig[i + ov] & MASK_12BIT, _adc_chars));
						}
						pDest[k] = pDest[k] / OVERSAMPLING;

						//apply hann window w[n]=0.5·(1-cos(2Pi·n/N))=sin^2(Pi·n/N)
						//auto hann = 0.5f * (1 - cos((2 * PI * k) / AUDIO_DATA_OUT));
						//log_d("%d - %1.4f", i, hann);
						//pInputFft[k] = hann * (float)pDest[k];
						pInputFft[k] = (float)pDest[k];

						//i ara escalem el valor
						pDest[k] = constrain(pDest[k], INPUT_0_VALUE - VOLATGE_DRAW_RANGE, INPUT_0_VALUE + VOLATGE_DRAW_RANGE);
						pDest[k] = map(pDest[k], INPUT_0_VALUE - VOLATGE_DRAW_RANGE, INPUT_0_VALUE + VOLATGE_DRAW_RANGE, 0, SCREEN_HEIGHT - 1);
					}
					uint16_t maxMagI=0;
					mad.pAudio = pDest;
					mad.pFftMag = _TaskParams.fftMag;
					theFFT.Execute();
					theFFT.GetFreqPower(mad.pFftMag, MAX_FFT_MAGNITUDE,
						BARS_RESOLUTION == 2 ? FftPower::ALL : BARS_RESOLUTION == 4 ? FftPower::HALF : FftPower::AUTO34,
						maxMagI, superMaxMag);
					mad.max_freq = (uint16_t)(maxMagI * freqs_x_bin);

					if(!xQueueSendToBack(_xQueSendAudio2Drawer, &mad, 0)) {
						log_d("Draw Queue FULL!!");
					}
					DrawLedBars(mad);
					//DrawLeds(mad);

					// if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
					// 	log_d("ShowLeds Queue FULL!!");
					// }
					auto now=millis();
					if((now-recInit)>=1000) {
						log_d("1sec receiving: time=%d totalSamples=%d numCalls=%d maxMag=%d", now - recInit, totalSamples, numCalls, superMaxMag);
						recInit=now;
						totalSamples=0;
						numCalls=0;
						superMaxMag=-10000;
					}
				}
			}
			// else if(adc_i2s_evt.type == I2S_EVENT_RX_Q_OVF) {
			// 	log_d("RX data dropped!!\n");
			// }
		}
	}
	vTaskDelete(NULL);
}

//Task to draw the LEDS
void vTaskDrawLeds(void* pvParameters)
{
	log_d("In vTaskDrawLeds...");
	MsgAudio2Draw mad;

	uint32_t lastShow=0;

	while(true) {
		if(xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY) && (millis()-lastShow)>100) {
			uint32_t init=millis();
			DrawLedBars(mad);
			lastShow=millis();
			//log_d("ShowTime=%dms", millis()-init);
		}
	}

	vTaskDelete(NULL);
}

// Task to draw screen.
void vTaskDrawer(void* pvParameters)
{
	log_d("In vTaskDrawer. Begin Display...");
	u8g2.setBusClock(BUS_SPEED);
	log_d("In vTaskDrawer.afterSetBus...");
	u8g2.begin();
	log_d("In vTaskDrawer.afterBegin...");
	u8g2.setFont(u8g2_font_profont12_mf);
	log_d("In vTaskDrawer.afterSetFont...");
	u8g2.setContrast(64);

	uint8_t lastBuff=0;
	uint32_t samplesDrawn=0;
	MsgAudio2Draw mad;

	while(true)
	{
		if(xQueueReceive(_xQueSendAudio2Drawer, &mad, (portTickType)portMAX_DELAY)) {
			uint16_t* pDest = mad.pAudio;

			if(_numFrames == 0) {
				_InitTime = millis();
			}

			//_TaskParams.newDataReady = false;
			if(lastBuff == _TaskParams.lastBuffSet) {
				log_d("Buff repeat! :(");
			}
			lastBuff = _TaskParams.lastBuffSet;
			// log_d("DataReady! DataBuffer=[%d]. BuffNumber=[%d]", _TaskParams.lastBuffSet, _TaskParams.buffNumber);
			u8g2.clearBuffer();

			//Now the FFT
			int16_t value = 0;

			// u8g2.setDrawColor(1);
			// uint16_t maxIndex = AUDIO_DATA_OUT / BARS_RESOLUTION; // /2->ALL /4->HALF /8->QUARTER
			// uint8_t barW = BARS_RESOLUTION == 2 ? 1 : BARS_RESOLUTION == 4 ? 2 : 4;
			// uint8_t adjust = BARS_RESOLUTION == 2?0:1;
			// uint8_t maxBassValue=0;
			// for(uint16_t i = 1; i < maxIndex; i++) {
			// 	value = constrain(mad.pFftMag[i], MIN_FFT_DB, MAX_FFT_DB);
			// 	value = map(value, MIN_FFT_DB, MAX_FFT_DB, 0, SCREEN_HEIGHT - 1);
			// 	// u8g2.drawLine(i, SCREEN_HEIGHT - value, i, SCREEN_HEIGHT-1); --> ALL
			// 	// u8g2.drawLine(i*2, SCREEN_HEIGHT - value, i*2, SCREEN_HEIGHT-1); --> HALF
			// 	u8g2.drawBox(i * barW, SCREEN_HEIGHT - value, barW - adjust, value); //--> QUARTER/AUTO
			// 	if(i<4 && value>maxBassValue) { //BASS bins
			// 		maxBassValue=value;
			// 	}
			// }
			// if(maxBassValue > (SCREEN_HEIGHT - (SCREEN_HEIGHT/4))) {
			// 	//u8g2.setContrast(255);
			// 	_BassOn=true;
			// 	// digitalWrite(PIN_BASS_LED, HIGH);
			// }
			// else {
			// 	_BassOn=false;
			// 	// digitalWrite(PIN_BASS_LED, LOW);
			// 	//u8g2.setContrast(64);
			// 	//u8g2.setContrast(map(maxBassValue, 0, SCREEN_HEIGHT - 1, 1, 128));
			// }
			// //now we simulate that the bars are made of "boxes"
			// u8g2.setDrawColor(0);
			// for(uint16_t i = 4; i < SCREEN_HEIGHT; i += 6) {
			// 	u8g2.drawLine(0, i, SCREEN_WIDTH - 1, i);
			// 	u8g2.drawLine(0, i + 1, SCREEN_WIDTH - 1, i + 1);
			// 	//u8g2.drawLine(0, i + 2, SCREEN_WIDTH - 1, i + 2);
			// }

			//Now The Wave
			u8g2.setDrawColor(2);

			//busquem el pass per "0" després de la muntanya més gran
			uint16_t pas0=0;
			uint16_t maxAmp = (SCREEN_HEIGHT / 2);
			uint16_t iMaxAmp=0;
			for(int i = 0; i < (AUDIO_DATA_OUT - SCREEN_WIDTH - SCREEN_WIDTH/3); i++) {
				if(pDest[i] < maxAmp) {
					maxAmp = value;
					iMaxAmp=i;
					break;
				}
			} //ja tenim l'index del pic de la muntanya mes gran. Ara busquem a on creuem per 0
			for(int i = iMaxAmp; i < (AUDIO_DATA_OUT - SCREEN_WIDTH); i++) {
				if(pDest[i] >= (SCREEN_HEIGHT / 2)) {
					pas0=i;
					break;
				}
			}
			for(uint16_t i = pas0; i < (pas0 + SCREEN_WIDTH); i++) {
				 value = constrain(pDest[i], INPUT_0_VALUE - VOLATGE_DRAW_RANGE, INPUT_0_VALUE + VOLATGE_DRAW_RANGE);
				 value = map(value, INPUT_0_VALUE - VOLATGE_DRAW_RANGE, INPUT_0_VALUE + VOLATGE_DRAW_RANGE, 0, SCREEN_HEIGHT - 1);
				 u8g2.drawPixel(i - pas0, pDest[i]);
			}
			samplesDrawn += AUDIO_DATA_OUT;

			//And finally the text
			u8g2.setFontMode(1);
			u8g2.setDrawColor(2);
			u8g2.drawStr(5, 15, Utils::string_format("FPS=%3.2f  F=%04dHz", _fps, mad.max_freq).c_str());

			u8g2.sendBuffer();
			_numFrames++;

			// if(!xQueueSendToBack(_xQueSendFft2Led, &mad, 0)) {
			// 	log_d("ShowLeds Queue FULL!!");
			// }
			// DrawLeds(mad);

			auto now=millis();
			if((now - _InitTime) >= 1000) {
				_fps = 1000.0f / ((millis() - _InitTime) / _numFrames);
				log_d("time=%d frames=%d fps=%3.2f samplesDrawn=%d", now - _InitTime, _numFrames, _fps, samplesDrawn);
				samplesDrawn = 0;
				_numFrames = 0;
			}
		}
	}
}

void setup()
{
	Serial.begin(115200);
	// wait for serial monitor to open
	while(!Serial);

	pinMode(PIN_BASS_LED, OUTPUT);
	pinMode(PIN_DATA_LEDS, OUTPUT);

	FastLED.addLeds<WS2812B, PIN_DATA_LEDS, GRB>(_TheLeds, NUM_LEDS);
	FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIS);
	_ThePanel.SetParams(&_TheLeds, &_TheMapping);

	//_TheLeds.fill_rainbow(HSVHue::HUE_YELLOW);

	//range 0...4096
	adc1_config_width(ADC_WIDTH_BIT_12);
	//full voltage range
	adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, _adc_chars);
		//Check type of calibration value used to characterize ADC
	if(val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
		log_d("calibration=eFuse Vref");
	}
	else if(val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
		log_d("calibration=Two Point");
	}
	else {
		log_d("calibration=Default");
	}

	// esp_err_t status = adc_vref_to_gpio(ADC_UNIT_1, ADC1_CHANNEL_4_GPIO_NUM);
	// if(status == ESP_OK) {
	// 	printf("v_ref routed to GPIO\n");
	// }
	// else {
	// 	printf("failed to route v_ref\n");
	// }
	log_d("Installing Driver...");
	i2s_config_t i2s_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
		.sample_rate = SAMPLE_RATE,//samplerate configured are the number of samples returned if channel=Left+Right, not frequency. So we need to multiply*2.
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_I2S_MSB, // I2S_COMM_FORMAT_STAND_MSB,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,  // Interrupt level 1, default 0
		.dma_buf_count = 2, //8,
		.dma_buf_len = (AUDIO_DATA_OUT * OVERSAMPLING), //AUDIO_DATA_OUT*OVERSAMPLING, //these are the number of samples we will read from i2s_read
		.use_apll = false,
		.tx_desc_auto_clear = false,
		.fixed_mclk = 0 //,//SAMPLE_RATE,
		// .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
		// .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
	};
	//install and start i2s driver
	auto err = i2s_driver_install(I2S_NUM_0, &i2s_config, _adc_i2s_event_queue_size, &_adc_i2s_event_queue);
	if(err!=ESP_OK) {
		log_d("driver install failed");
	}
	//i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
	static const i2s_pin_config_t pin_config = {
	.bck_io_num = I2S_PIN_NO_CHANGE,							// Sample f(Hz) (= sample f * 2)  on this pin (optional).
	.ws_io_num = I2S_PIN_NO_CHANGE,							// Left/Right   (= sample f)      on this pin (optional).
	.data_out_num = I2S_PIN_NO_CHANGE,
	.data_in_num = RTCIO_CHANNEL_4_GPIO_NUM//ADC1_CHANNEL_4_GPIO_NUM// RTCIO_CHANNEL_4_GPIO_NUM
	};
	err= i2s_set_pin(I2S_NUM_0, &pin_config);
	if(err != ESP_OK) {
		log_d("i2s_set_pin failed");
	}
	//init ADC pad
	err=i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_4);
	if(err != ESP_OK) {
		log_d("set_adc_mode failed");
	}
	// //enable de ADC
	 err=i2s_adc_enable(I2S_NUM_0);

	//Create queue to communicate reader & drawer
	 _xQueSendAudio2Drawer = xQueueCreate(1, sizeof(MsgAudio2Draw));
	 _xQueSendFft2Led = xQueueCreate(1, sizeof(MsgAudio2Draw));
	//start task to read samples from I2S
	 xTaskCreate(vTaskReader, "ReaderTask", 2048, (void*)&_TaskParams, 2, &_readerTaskHandle);
	 //xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 2048, (void*)&_TaskParams, 2, &_readerTaskHandle, 0);
	//start task to draw screen
	xTaskCreate(vTaskDrawer, "Draw Screen", 2048, (void*)&_TaskParams, 2, &_drawTaskHandle);
	//xTaskCreatePinnedToCore(vTaskDrawer, "Draw Screen", 2048, (void*)&_TaskParams, 2, &_drawTaskHandle, 1);
	//start task to draw leds
	//xTaskCreatePinnedToCore(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle, 0);
//	xTaskCreate(vTaskDrawLeds, "Draw Leds", 2048, (void*)&_TaskParams, 2, &_showLedsTaskHandle);
}

// uint8_t thisSpeed = 3;
// uint8_t initial = 1;
void loop()
{
	// if(_BassOn) {
	// 	digitalWrite(PIN_BASS_LED, HIGH);
	// }
	// else {
	// 	digitalWrite(PIN_BASS_LED, LOW);
	// }

	// fill_rainbow(_TheLeds, NUM_LEDS, initial, thisSpeed);
	// initial+=thisSpeed;
	// FastLED.show();
	delay(100);
}

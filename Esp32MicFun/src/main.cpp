#include <Arduino.h>
#include <U8g2lib.h>
#include <memory>
#include "SharedUtils\Utils.h"
#include "fft.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
//#include <components/freertos/FreeRTOS-Kernel/include/freertos/queue.h>

#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define DEFAULT_VREF       1100
#define INPUT_0_VALUE      2100  //input is biased towards 2.1V (dc value)
#define VOLATGE_DRAW_RANGE 250   //400  //total range is this value*2. in millivolts. 400 imply a visible range from [INPUT_0_VALUE-400]....[INPUT_0_VALUE+400]
#define MIN_FFT_MAGNITUDE  200   //a magnitude under this value will be considered 0 (noise)
#define MAX_FFT_MAGNITUDE  7000  //a magnitude greater than this value will be considered Max Power
#define SAMPLE_RATE        10000 //we will oversample by 2. We can only draw up to 5kpixels per second

#define MASK_12BIT 0x0fff

//U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, PIN_I2C_SCL, PIN_I2C_SDA);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 255, PIN_I2C_SCL, PIN_I2C_SDA);

uint8_t _ScreenBrightness=0;
uint32_t _InitTime=0;
uint8_t  _numFrames = 0;
float _fps=0.0f;


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

QueueHandle_t _adc_i2s_event_queue;
uint8_t _adc_i2s_event_queue_size = 1;


struct TaskParams {
	uint16_t data1[SCREEN_WIDTH];
	uint16_t data2[SCREEN_WIDTH];
	int32_t fftMag[SCREEN_WIDTH / 2];
	uint16_t dataOrig[SCREEN_WIDTH * 2];
	uint8_t lastBuffSet; //0=none, 1=data1, 2=data2
	bool newDataReady;
	uint32_t buffNumber;

	TaskParams() {
		lastBuffSet=0;
		newDataReady=false;
		buffNumber=0;
	}
};
TaskParams _TaskParams;

// Task to read samples.
void vTaskReader(void* pvParameters)
{
	size_t buffSize = sizeof(_TaskParams.data1);
	size_t buffSizeOrig = sizeof(_TaskParams.dataOrig);
	size_t bytesRead=0;
	log_d("Estic a la ReaderTask. BuffSize=[%d]", buffSize);
	i2s_event_t adc_i2s_evt;
	uint32_t recInit=millis();
	uint32_t totalData=0;
	uint16_t numCalls=0;
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
					totalData += bytesRead;
					numCalls++;
					_TaskParams.buffNumber++;
					//now we convert the values to mv from 1V to 3V
					for(int i = 0, k=0; i < bytesRead; i+=2, k++) {
							//pDest[i] = (uint16_t)(esp_adc_cal_raw_to_voltage(pDest[i] & MASK_12BIT, _adc_chars) - (INPUT_0_VALUE / 2));
						pDest[k] = (uint16_t)(esp_adc_cal_raw_to_voltage(_TaskParams.dataOrig[i] & MASK_12BIT, _adc_chars));
						pDest[k] += (uint16_t)(esp_adc_cal_raw_to_voltage(_TaskParams.dataOrig[i+1] & MASK_12BIT, _adc_chars));
						pDest[k] = pDest[k]/2;
							//pDest[i] = (uint16_t)pDest[i] & MASK_12BIT;
						//log_d("%d", pDest[i]);
					}
					// uint32_t avg = ((pDest[0] & MASK_12BIT) + (pDest[1] & MASK_12BIT) + (pDest[2] & MASK_12BIT)) / 3;
					// log_d("ReadNumber [%d]. Buff=[%d]. Read [%d] bytes.", _TaskParams.buffNumber, _TaskParams.lastBuffSet, bytesRead);
					_TaskParams.newDataReady = true;
					auto now=millis();
					if((now-recInit)>=1000) {
						log_d("1sec receiving: time=%d totalData=%d numCalls=%d", now - recInit, totalData, numCalls);
						recInit=now;
						totalData=0;
						numCalls=0;
					}
				}
			}
		}
	}
	vTaskDelete(NULL);
}

// Task to draw screen.
void vTaskDrawer(void* pvParameters)
{
	log_d("In vTaskDrawer. Begin Display...");
	u8g2.setBusClock(700000);
	log_d("In vTaskDrawer.afterSetBus...");
	u8g2.begin();
	log_d("In vTaskDrawer.afterBegin...");
	u8g2.setFont(u8g2_font_profont12_mf);
	log_d("In vTaskDrawer.afterSetFont...");

			// Create the FFT config structure
	fft_config_t* real_fft_plan = fft_init(SCREEN_WIDTH, FFT_REAL, FFT_FORWARD, NULL, NULL);
	float freqs_x_bin=(float)(SAMPLE_RATE/2)/(float)SCREEN_WIDTH;
	log_d("Freqs_x_Bin=%3.2f", freqs_x_bin);

	uint8_t lastBuff=0;
	uint32_t samplesDrawn=0;
	while(true)
	{
		if(_TaskParams.newDataReady) {
			uint16_t* pDest = _TaskParams.lastBuffSet == 1 ? _TaskParams.data1 : _TaskParams.data2;

			if(_numFrames == 0) {
				_InitTime = millis();
			}

			//TEEEEST FFT
			for(int i=0; i<SCREEN_WIDTH; i++) {
				real_fft_plan->input[i]=(float)pDest[i];
			}
			fft_execute(real_fft_plan);
			int32_t maxMag=0;
			uint16_t maxMagI=0;
			for(int i = 1; i < SCREEN_WIDTH/2; i++) {
				_TaskParams.fftMag[i] = (int32_t)sqrt(pow(real_fft_plan->output[i * 2], 2) + pow(real_fft_plan->output[(i * 2)+1], 2));

				//log_d("Freq=%3.2f Magn=%3.2f", i * freqs_x_bin, _TaskParams.fftMag[i]);

				if(_TaskParams.fftMag[i]>maxMag) {
					maxMag = _TaskParams.fftMag[i];
					maxMagI=i;
				}
			}
			// if(maxMag>MIN_FFT_MAGNITUDE) {
			// 	log_d("MaxFreq=%3.2f Magn=%d", maxMagI * freqs_x_bin, maxMag);
			// }
			//FI TEEEEST FFT

			_TaskParams.newDataReady = false;
			if(lastBuff == _TaskParams.lastBuffSet) {
				log_d("Buff repeat! :(");
			}
			lastBuff = _TaskParams.lastBuffSet;
			// log_d("DataReady! DataBuffer=[%d]. BuffNumber=[%d]", _TaskParams.lastBuffSet, _TaskParams.buffNumber);
			u8g2.clearBuffer();

			uint16_t value=0;
			for(int i=0; i<SCREEN_WIDTH; i++) {
				value = constrain(pDest[i], INPUT_0_VALUE - VOLATGE_DRAW_RANGE, INPUT_0_VALUE + VOLATGE_DRAW_RANGE);
				value = map(value, INPUT_0_VALUE - VOLATGE_DRAW_RANGE, INPUT_0_VALUE + VOLATGE_DRAW_RANGE, 0, SCREEN_HEIGHT - 1);
				u8g2.drawPixel(i, value);
			}
			samplesDrawn+=SCREEN_WIDTH;

			//Now the FFT
			u8g2.setDrawColor(2);
			for(int i = 1; i < SCREEN_WIDTH / 2; i++) {
				value = constrain(_TaskParams.fftMag[i], MIN_FFT_MAGNITUDE, 8000);
				value = map(value, MIN_FFT_MAGNITUDE, 8000, 0, SCREEN_HEIGHT - 1);
				u8g2.drawFrame(i * 2, SCREEN_HEIGHT - value, 2, value);
			}

			u8g2.setFontMode(1);
//			u8g2.setDrawColor(2);
			u8g2.drawStr(5, 15, Utils::string_format("FPS=%3.2f  F=%04dHz", _fps, maxMag > MIN_FFT_MAGNITUDE ? (int32_t)(maxMagI * freqs_x_bin) : 0).c_str());
			u8g2.sendBuffer();
			_numFrames++;

			auto now=millis();
			if((now - _InitTime) >= 1000) {
				_fps = 1000.0f / ((millis() - _InitTime) / _numFrames);
				log_d("time=%d frames=%d fps=%3.2f samplesDrawn=%d", now - _InitTime, _numFrames, _fps, samplesDrawn);
				samplesDrawn = 0;
				_numFrames = 0;
			}
		}
		else {
			vTaskDelay(1);
		}
		// u8g2.setContrast(_ScreenBrightness);
		// u8g2.clearBuffer();

		// _PosX += _incX;
		// if(_PosX >= (SCREEN_WIDTH - _ballRadius) && _incX > 0) {
		// 	_PosX = SCREEN_WIDTH - _ballRadius;
		// 	_incX = (-_incX);
		// 	_incRadius = (-_incRadius);
		// }
		// else if(_PosX <= _ballRadius && _incX < 0) {
		// 	_PosX = _ballRadius;
		// 	_incX = (-_incX);
		// 	_incRadius = (-_incRadius);
		// }
		// _PosY += _incY;
		// if(_PosY >= (SCREEN_HEIGHT - _ballRadius) && _incY > 0) {
		// 	_PosY = SCREEN_HEIGHT - _ballRadius;
		// 	_incY = (-_incY);
		// }
		// else if(_PosY <= _ballRadius && _incY < 0) {
		// 	_PosY = _ballRadius;
		// 	_incY = (-_incY);
		// }

		// //log_d("x=%d, y=%d, r=%d", _PosX, _PosY, _ballRadius);

		// u8g2.setDrawColor(1);
		// u8g2.drawDisc(_PosX, _PosY, _ballRadius);

		// _ballRadius += _incRadius;
		// if(_ballRadius < 1) {
		// 	_ballRadius = 1;
		// }

		// u8g2.setFontMode(1);
		// u8g2.setDrawColor(2);
		// u8g2.drawStr(20, 15, Utils::string_format("FPS=%3.2f", _fps).c_str());

		// u8g2.sendBuffer();
		//_numFrames++;
	}
}

void setup()
{
	Serial.begin(115200);
	// wait for serial monitor to open
	while(!Serial);

	// log_d("Begin Display...");
	// u8g2.setBusClock(600000);
	// u8g2.begin();
	// u8g2.setFont(u8g2_font_profont10_mf);

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
		.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,  // Interrupt level 1, default 0
		.dma_buf_count = 8,
		.dma_buf_len = SCREEN_WIDTH*2, //these are the number of samples we will read from i2s_read
		.use_apll = false,
		.tx_desc_auto_clear = false
		//.fixed_mclk = SAMPLE_RATE
	};
	//install and start i2s driver
	auto err = i2s_driver_install(I2S_NUM_0, &i2s_config, _adc_i2s_event_queue_size, &_adc_i2s_event_queue);
	if(err!=ESP_OK) {
		log_d("driver install failed");
	}
	static const i2s_pin_config_t pin_config = {
	.bck_io_num = I2S_PIN_NO_CHANGE,							// Sample f(Hz) (= sample f * 2)  on this pin (optional).
	.ws_io_num = I2S_PIN_NO_CHANGE,							// Left/Right   (= sample f)      on this pin (optional).
	.data_out_num = I2S_PIN_NO_CHANGE,
	.data_in_num = ADC1_CHANNEL_4_GPIO_NUM
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

	//start task to read samples from I2S
	xTaskCreatePinnedToCore(vTaskReader, "ReaderTask", 8192, (void*)&_TaskParams, 1, &_readerTaskHandle, 1);
	//start task to draw screen
	xTaskCreatePinnedToCore(vTaskDrawer, "Draw Screen", 8192, (void*)&_TaskParams, 1, &_drawTaskHandle, 0);
}

void DrawScreen()
{
//	u8g2.setContrast(_ScreenBrightness);
	u8g2.clearBuffer();

	_PosX += _incX;
	if(_PosX >= (SCREEN_WIDTH - _ballRadius) && _incX > 0) {
		_PosX = SCREEN_WIDTH - _ballRadius;
		_incX=(-_incX);
		_incRadius= (-_incRadius);
	}
	else if(_PosX <= _ballRadius && _incX<0) {
		_PosX = _ballRadius;
		_incX = (-_incX);
		_incRadius = (-_incRadius);
	}
	_PosY += _incY;
	if(_PosY >= (SCREEN_HEIGHT - _ballRadius) && _incY > 0) {
		_PosY = SCREEN_HEIGHT - _ballRadius;
		_incY = (-_incY);
	}
	else if(_PosY <= _ballRadius && _incY<0) {
		_PosY = _ballRadius;
		_incY = (-_incY);
	}

	//log_d("x=%d, y=%d, r=%d", _PosX, _PosY, _ballRadius);

	u8g2.setDrawColor(1);
	u8g2.drawDisc(_PosX, _PosY, _ballRadius);

	_ballRadius+=_incRadius;
	if(_ballRadius<1) {
		_ballRadius=1;
	}

	// u8g2.setDrawColor(1);
	// for(int y = 0; y < SCREEN_HEIGHT; y++) {
	// 	for(int x = y+_numFrames; x < SCREEN_WIDTH; x+=10) {
	// 		u8g2.drawPixel(x, y);
	// 	}
	// }
	u8g2.setFontMode(1);
	u8g2.setDrawColor(2);
	u8g2.drawStr(20, 15, Utils::string_format("FPS=%3.2f", _fps).c_str());
	// u8g2.setFont(u8g2_font_profont10_mf);
	// u8g2.drawStr(0, 6, "Soc Peque 6px");
	// u8g2.setFont(u8g2_font_ncenB14_tr);
	// u8g2.drawStr(0, 24, "Hola Bola!");
	// u8g2.setFont(u8g2_font_tom_thumb_4x6_mf);
	// u8g2.drawStr(0, 30, "Soc Peque 5px");
	// u8g2.setFont(u8g2_font_p01type_tf);
	// u8g2.drawStr(64, 30, "Soc Peque 4px");
	// u8g2.setFont(u8g2_font_open_iconic_check_2x_t);
	// u8g2.drawStr(40 -_numFrames * 2, 30 + _numFrames * 3, "ABCDE");
	// u8g2.drawPixel(_numFrames * 10, 35);
	// u8g2.drawPixel((_numFrames * 10) + 1, 35+1);
	// u8g2.drawPixel((_numFrames * 10) + 2, 35+2);
	// u8g2.drawPixel((_numFrames * 10) + 3, 35 + 3);
	// u8g2.drawPixel((_numFrames * 10) + 4, 35 + 4);
	u8g2.sendBuffer();
}

void loop()
{
	// if(_numFrames==10) {
	// 	_fps=1000.0f/((millis()-_InitTime)/10);
	// 	 //log_d("time=%d fps=%3.2f", millis() - _InitTime, _fps);
	// 	_numFrames=0;
	// }
	// if(_numFrames==0) {
	// 	_InitTime=millis();
	// }

	//DrawScreen();

	// uint32_t reading = adc1_get_raw(ADC1_CHANNEL_4);
	// uint32_t voltage = esp_adc_cal_raw_to_voltage(reading, _adc_chars);
// 	if(_TaskParams.newDataReady) {
// 		_TaskParams.newDataReady = false;
// //		log_d("DataReady! DataBuffer=[%d]. BuffNumber=[%d]", _TaskParams.lastBuffSet, _TaskParams.buffNumber);
// 	}

	// log_d("r=%d v=%d", reading, voltage);
	//delay(1000);
}

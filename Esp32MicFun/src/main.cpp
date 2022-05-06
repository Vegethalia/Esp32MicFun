#include <Arduino.h>
#include <U8g2lib.h>
#include "SharedUtils\Utils.h"

#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

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


void setup()
{
	Serial.begin(115200);
	// wait for serial monitor to open
	while(!Serial);

	log_d("Begin Display...");
	u8g2.setBusClock(600000);
	u8g2.begin();
	u8g2.setFont(u8g2_font_profont10_mf);

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
//	u8g2.setFontMode(1);
//	u8g2.setDrawColor(2);
	//u8g2.drawStr(0, 6, "Soc Peque 6px");
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
	if(_numFrames==10) {
		_fps=1000.0f/((millis()-_InitTime)/10);
		log_d("time=%d fps=%3.2f", millis() - _InitTime, _fps);
		_numFrames=0;
	}
	if(_numFrames==0) {
		_InitTime=millis();
	}

	DrawScreen();

	_numFrames++;
	//delay(1000);
}

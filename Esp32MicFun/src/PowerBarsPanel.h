#pragma once

#include <string>
#include <FastLED.h>
constexpr uint16_t PANEL_WIDTH_33 = 33; //from 1 to 33
constexpr uint16_t PANEL_HEIGHT_9 = 9; //from 1 to 9

template<uint16_t PANEL_WIDTH, uint16_t PANEL_HEIGHT>
class IPanelMapping
{
public:
	//Implements the mapping between the passed (x,y) coordinates and the serpentine led_strip
	virtual uint16_t XY(uint16_t x, uint16_t y) = 0;
	//returns the dimension of this mapping
	virtual uint16_t GetWidth() { return PANEL_WIDTH_33; }
	virtual uint16_t GetHeight() { return PANEL_HEIGHT_9; }
};

//Mapping implementation for the 33x9 panel
class PanelMapping33x9 : public IPanelMapping<PANEL_WIDTH_33, PANEL_HEIGHT_9>
{
public:
	virtual uint16_t XY(uint16_t x, uint16_t y) override
	{
		//mapping created using the https://macetech.github.io/FastLED-XY-Map-Generator/
		static constexpr uint16_t XYTable[PANEL_WIDTH_33 * PANEL_HEIGHT_9] = {
		 296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,
		 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,
		 230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198,
		 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
		 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132,
			99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
			98,  97,  96,  95,  94,  93,  92,  91,  90,  89,  88,  87,  86,  85,  84,  83,  82,  81,  80,  79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,  68,  67,  66,
			33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,
			32,  31,  30,  29,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0
		};
			// any out of bounds address maps to the first hidden pixel
		if((x >= PANEL_WIDTH_33) || (y >= PANEL_HEIGHT_9)) {
			return (PANEL_WIDTH_33 * PANEL_HEIGHT_9);
		}

		uint16_t i = (y * PANEL_WIDTH_33) + x;
		uint16_t j = XYTable[i];
		return j;
	}
};

//This class encapsulates the whole drawing process in the "serpentine led panel", made of PANEL_WIDTH x PANEL_HEIGHT leds (or pixels)
template<uint16_t TOTAL_LEDS, uint16_t PANEL_WIDTH, uint16_t PANEL_HEIGHT>
class PowerBarsPanel
{
private:
	struct Column {
		uint8_t CurrentTop;
		uint8_t CurrentHeight;
		uint32_t LastHeightDecreaseTime;

		Column() {
			CurrentTop=0;
			CurrentHeight=0;
			LastHeightDecreaseTime=0;
		}
	};

public:
	PowerBarsPanel() {};
	virtual ~PowerBarsPanel() {};

	//Configure the class basic params: the FastLed object and the dimensions of the panel.
	//The only valid dimension so far is  PANEL_WIDTH x PANEL_HEIGHT.
	//Returns false if any of teh params is not valid.
	bool SetParams(CRGBArray<TOTAL_LEDS>* pTheLeds, IPanelMapping<PANEL_WIDTH, PANEL_HEIGHT>* pTheMapping)
	{
		if(TOTAL_LEDS < (PANEL_WIDTH * PANEL_HEIGHT)) {
			return false;
		}
		if(!pTheLeds || !pTheMapping || pTheMapping->GetWidth() != PANEL_WIDTH || pTheMapping->GetHeight() != PANEL_HEIGHT) {
			return false;
		}
		_pTheLeds = pTheLeds;
		_pTheMapping = pTheMapping;

		return true;
	}

	//Draws the n-th bar (0 based) with the value passed. Value must be scaled from 0 to PANEL_HEIGHT*10
	void DrawBar(uint8_t numBar, uint8_t value)
	{
		constexpr uint16_t maxheight = PANEL_HEIGHT-1;
		if(!_pTheLeds || !_pTheMapping) {
			return;
		}

		bool isMax = value > (maxheight * 10);

		if(value < 5) {
			(*_pTheLeds)[_pTheMapping->XY(numBar, maxheight)] = CHSV(HSVHue::HUE_BLUE + numBar * 0, 255, 12);
		}
		else {
			uint8_t y = 0;
			while(y <= value / 10) {
				(*_pTheLeds)[_pTheMapping->XY(numBar, maxheight - y)] = CHSV(HSVHue::HUE_BLUE + (y * 18) + numBar * 0, 255, (y + 1) * 15);
				++y;
			};
			if(isMax) {
				y = maxheight;
				(*_pTheLeds)[_pTheMapping->XY(numBar, maxheight - y)] = CHSV(HSVHue::HUE_AQUA, 255, 15 * (maxheight + 1));
			}
		}
	}

private:
	CRGBArray<TOTAL_LEDS>* _pTheLeds; //The FastLed object
	IPanelMapping<PANEL_WIDTH, PANEL_HEIGHT>* _pTheMapping; //The class that will provide the mapping coordinates
	Column _TheColumns[PANEL_WIDTH];
};

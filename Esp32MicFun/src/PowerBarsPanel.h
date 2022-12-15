#pragma once

#include <FastLED.h>
#include <string>
constexpr uint16_t PANEL_WIDTH_33 = 33; // from 1 to 33
constexpr uint16_t PANEL_HEIGHT_9 = 9; // from 1 to 9
constexpr uint16_t PANEL_HEIGHT_12 = 12; // from 1 to 12
constexpr uint16_t PANEL_HEIGHT_13 = 13; // from 1 to 13
constexpr uint16_t PANEL_HEIGHT_16 = 16; // from 1 to 13

constexpr uint16_t BAR_DEC_TIME_MS = 50; // hold each bar max current value at least this amount of time
constexpr uint16_t BAR_TOP_TIME_MS = BAR_DEC_TIME_MS * 10; // hold the top for this amount

template <uint16_t PANEL_WIDTH, uint16_t PANEL_HEIGHT>
class IPanelMapping {
public:
    // Implements the mapping between the passed (x,y) coordinates and the serpentine led_strip
    virtual uint16_t XY(uint16_t x, uint16_t y) = 0;
    // returns the dimension of this mapping
    virtual uint16_t GetWidth() { return PANEL_WIDTH; }
    virtual uint16_t GetHeight() { return PANEL_HEIGHT; }
};

// Mapping implementation for the 33x9 panel
class PanelMapping33x9 : public IPanelMapping<PANEL_WIDTH_33, PANEL_HEIGHT_9> {
public:
    virtual uint16_t XY(uint16_t x, uint16_t y) override
    {
        // mapping created using the https://macetech.github.io/FastLED-XY-Map-Generator/
        static constexpr uint16_t XYTable[PANEL_WIDTH_33 * PANEL_HEIGHT_9] = {
            296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,
            231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,
            230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198,
            165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
            164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132,
            99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
            98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
            32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };
        // any out of bounds address maps to the first hidden pixel
        if ((x >= PANEL_WIDTH_33) || (y >= PANEL_HEIGHT_9)) {
            return (PANEL_WIDTH_33 * PANEL_HEIGHT_9);
        }

        uint16_t i = (y * PANEL_WIDTH_33) + x;
        uint16_t j = XYTable[i];
        return j;
    }
};

// Mapping implementation for the 33x12 panel
class PanelMapping33x12 : public IPanelMapping<PANEL_WIDTH_33, PANEL_HEIGHT_12> {
public:
    virtual uint16_t XY(uint16_t x, uint16_t y) override
    {
        // mapping created using the https://macetech.github.io/FastLED-XY-Map-Generator/
        static constexpr uint16_t XYTable[PANEL_WIDTH_33 * PANEL_HEIGHT_12] = {
            363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395,
            362, 361, 360, 359, 358, 357, 356, 355, 354, 353, 352, 351, 350, 349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330,
            297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
            296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,
            231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,
            230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198,
            165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
            164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132,
            99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
            98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
            32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };
        // any out of bounds address maps to the first hidden pixel
        if ((x >= PANEL_WIDTH_33) || (y >= PANEL_HEIGHT_12)) {
            return (PANEL_WIDTH_33 * PANEL_HEIGHT_12);
        }

        uint16_t i = (y * PANEL_WIDTH_33) + x;
        uint16_t j = XYTable[i];
        return j;
    }
};

// Mapping implementation for the 33x13 panel
class PanelMapping33x13 : public IPanelMapping<PANEL_WIDTH_33, PANEL_HEIGHT_13> {
public:
    virtual uint16_t XY(uint16_t x, uint16_t y) override
    {
        // mapping created using the https://macetech.github.io/FastLED-XY-Map-Generator/
        static constexpr uint16_t XYTable[PANEL_WIDTH_33 * PANEL_HEIGHT_13] = {
            428, 427, 426, 425, 424, 423, 422, 421, 420, 419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403, 402, 401, 400, 399, 398, 397, 396,
            363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395,
            362, 361, 360, 359, 358, 357, 356, 355, 354, 353, 352, 351, 350, 349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330,
            297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
            296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,
            231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,
            230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198,
            165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
            164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132,
            99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
            98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
            32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };
        // any out of bounds address maps to the first hidden pixel
        if ((x >= PANEL_WIDTH_33) || (y >= PANEL_HEIGHT_13)) {
            return (PANEL_WIDTH_33 * PANEL_HEIGHT_13);
        }

        uint16_t i = (y * PANEL_WIDTH_33) + x;
        uint16_t j = XYTable[i];
        return j;
    }
};

// Mapping implementation for the 33x16 panel
class PanelMapping33x16 : public IPanelMapping<PANEL_WIDTH_33, PANEL_HEIGHT_16> {
public:
    virtual uint16_t XY(uint16_t x, uint16_t y) override
    {
        // mapping created using the https://macetech.github.io/FastLED-XY-Map-Generator/
        static constexpr uint16_t XYTable[PANEL_WIDTH_33 * PANEL_HEIGHT_16] = {
            495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527,
            494, 493, 492, 491, 490, 489, 488, 487, 486, 485, 484, 483, 482, 481, 480, 479, 478, 477, 476, 475, 474, 473, 472, 471, 470, 469, 468, 467, 466, 465, 464, 463, 462,
            429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461,
            428, 427, 426, 425, 424, 423, 422, 421, 420, 419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403, 402, 401, 400, 399, 398, 397, 396,
            363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395,
            362, 361, 360, 359, 358, 357, 356, 355, 354, 353, 352, 351, 350, 349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330,
            297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
            296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264,
            231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263,
            230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198,
            165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
            164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132,
            99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
            98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
            32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };
        // any out of bounds address maps to the first hidden pixel
        if ((x >= PANEL_WIDTH_33) || (y >= PANEL_HEIGHT_16)) {
            return (PANEL_WIDTH_33 * PANEL_HEIGHT_16);
        }

        uint16_t i = (y * PANEL_WIDTH_33) + x;
        uint16_t j = XYTable[i];
        return j;
    }
};

// This class encapsulates the whole drawing process in the "serpentine led panel", made of PANEL_WIDTH x PANEL_HEIGHT leds (or pixels)
template <uint16_t TOTAL_LEDS, uint16_t PANEL_WIDTH, uint16_t PANEL_HEIGHT>
class PowerBarsPanel {
private:
    struct Column {
        uint8_t CurrentTop;
        uint8_t CurrentHeight;
        uint32_t LastHeightDecreaseTime;
        uint32_t LastTopSetTime;

        Column()
        {
            CurrentTop = 0;
            CurrentHeight = 0;
            LastHeightDecreaseTime = 0;
            LastTopSetTime = 0;
        }
    };
    enum COLOR_SCHEME {
        NONE = 0,
        CS1 = 1,
        CS2 = 2, // basehue bars brighter if higher
        CS3 = 3 // White bars, brighter if higher
    };

public:
    PowerBarsPanel()
        : _CurrentBaseHue(HSVHue::HUE_AQUA)
        , _ColorScheme(COLOR_SCHEME::CS1) {};
    virtual ~PowerBarsPanel() {};

    // Sets the base hue for the color scheme
    void SetBaseHue(uint8_t theHue) { _CurrentBaseHue = theHue; }
    // Returns the current base hue
    uint8_t GetBaseHue() { return _CurrentBaseHue; }
    // Increments the base hue by the step value
    void IncBaseHue(uint8_t step) { _CurrentBaseHue += step; }

    // Configure the class basic params: the FastLed object and the dimensions of the panel.
    // The only valid dimension so far is  PANEL_WIDTH x PANEL_HEIGHT.
    // Returns false if any of teh params is not valid.
    bool SetParams(CRGBArray<TOTAL_LEDS>* pTheLeds, IPanelMapping<PANEL_WIDTH, PANEL_HEIGHT>* pTheMapping)
    {
        if (TOTAL_LEDS < (PANEL_WIDTH * PANEL_HEIGHT)) {
            return false;
        }
        if (!pTheLeds || !pTheMapping || pTheMapping->GetWidth() != PANEL_WIDTH || pTheMapping->GetHeight() != PANEL_HEIGHT) {
            return false;
        }
        _pTheLeds = pTheLeds;
        _pTheMapping = pTheMapping;

        return true;
    }

    // Draws the n-th bar (0 based) with the value passed. Value must be scaled from 0 to PANEL_HEIGHT*10
    void DrawBar(uint8_t numBar, uint8_t value, uint8_t brightness)
    {
        constexpr uint16_t maxheight = PANEL_HEIGHT - 1;
        if (!_pTheLeds || !_pTheMapping || numBar >= PANEL_WIDTH) {
            return;
        }

        auto now = millis();

        // uint8_t minBoostBin = (uint8_t)(PANEL_WIDTH*0.13); //the first 4 bars in 33 width panel
        // constexpr float maxTrebleBoost=1.8;
        // constexpr float minBassBoost = 1.0;
        // constexpr float freqBoost = ((maxTrebleBoost - minBassBoost) / (float)PANEL_WIDTH);

        // if(numBar>minBoostBin) {
        // 	auto boost = 1.0f + (numBar * freqBoost);
        // 	value = min((int)(value * boost), (PANEL_HEIGHT * 10)-1);
        // }

        if (_TheColumns[numBar].CurrentHeight < value) {
            _TheColumns[numBar].CurrentHeight = value;
            _TheColumns[numBar].LastHeightDecreaseTime = now;
        }
        if (_TheColumns[numBar].CurrentTop < value) {
            _TheColumns[numBar].CurrentTop = value;
            _TheColumns[numBar].LastTopSetTime = now;
        }

        if ((now - _TheColumns[numBar].LastHeightDecreaseTime) > BAR_DEC_TIME_MS) {
            if (_TheColumns[numBar].CurrentHeight >= 10) {
                _TheColumns[numBar].CurrentHeight -= 10;
            } else {
                _TheColumns[numBar].CurrentHeight = 0;
            }
            _TheColumns[numBar].LastHeightDecreaseTime = now;
        }
        if ((now - _TheColumns[numBar].LastTopSetTime) > BAR_TOP_TIME_MS) {
            _TheColumns[numBar].LastTopSetTime = now - (BAR_TOP_TIME_MS * 4 / 5);
            if (_TheColumns[numBar].CurrentTop >= 10) {
                _TheColumns[numBar].CurrentTop -= 10;
            } else {
                _TheColumns[numBar].CurrentTop = 0;
            }
            //_TheColumns[numBar].CurrentTop = _TheColumns[numBar].CurrentHeight;
        }

        value = _TheColumns[numBar].CurrentHeight;

        bool isMax = value > ((maxheight * 10) - 1);

        // 		if(value < 5 && _TheColumns[numBar].CurrentTop < 10) { //do not paint
        // //			(*_pTheLeds)[_pTheMapping->XY(numBar, maxheight)] = CHSV(HSVHue::HUE_BLUE + numBar * 0, 255, 15);// CRGB(2,2,1);//
        // 		}
        // 		else {
        uint8_t y = 0;
        uint8_t maxY = value / 10;
        if (value >= 5) {
            while (y <= maxY) {
                CRGB colPixel;
                switch (_ColorScheme) {
                case COLOR_SCHEME::CS2:
                    colPixel = CHSV(_CurrentBaseHue + (y * 18) + numBar * 6, 255, (y + 2) * 15);
                    break;
                case COLOR_SCHEME::CS1: {
                    // auto bright2 = (y)*10;
                    colPixel = CHSV(_CurrentBaseHue, 255, maxY * 10); //(y + 2) * 15); // CRGB(bright, bright, bright);
                    break;
                }
                default: {
                    // auto bright0 = maxY * 10;
                    CHSV white = rgb2hsv_approximate(CRGB::DarkRed); // CRGB::FairyLight
                    white.v = maxY * 3;
                    colPixel = white; // CRGB(maxY, maxY, maxY);
                    break;
                }
                }
                (*_pTheLeds)[_pTheMapping->XY(numBar, maxheight - y)] += colPixel;
                ++y;
            };
        }
        while (y <= maxheight) { // per pintar el "background"
            switch (_ColorScheme) {
            case COLOR_SCHEME::CS2:
                break;
            case COLOR_SCHEME::CS1:
            default:
                //(*_pTheLeds)[_pTheMapping->XY(numBar, maxheight - y)] = CRGB(1, 1, 1);
                break;
            }
            ++y;
        }
        // if(isMax) {
        // 	y = maxheight;
        // 	(*_pTheLeds)[_pTheMapping->XY(numBar, maxheight - y)] = CHSV(HSVHue::HUE_AQUA,                         255, 15 * (maxheight + 1));
        // }
        // }

        // Now the top
        if (_TheColumns[numBar].CurrentTop >= 10) {
            uint8_t y = min(uint16_t(_TheColumns[numBar].CurrentTop / 10), maxheight);
            CHSV colPixel;
            switch (_ColorScheme) {
            case COLOR_SCHEME::CS2:
                colPixel = CHSV(_CurrentBaseHue + (y * 18) + numBar * 6, 255, (y + 1) * 15);
                break;
            case COLOR_SCHEME::CS1:
                colPixel = CHSV(HSVHue::HUE_PINK, 255, 10 * maxheight);
                break;
            default:
                // auto bright = (y + 2) * 15;
                colPixel = CHSV(HSVHue::HUE_PINK, 255, 5 * maxheight);
                break;
            }
            (*_pTheLeds)[_pTheMapping->XY(numBar, maxheight - y)] = colPixel;
        }
        //_CurrentBaseHue++;
    }

    // shifts all bars to the left and inserts the new one. The values are interpreted as the intensity (V) in HSV
    // The array is supposed to hold PANEL_HEIGHT values
    void PushBar(const uint8_t* pTheValues)
    {
        int ledDest = 0;
        // shift all columns to the left
        for (int iBar = 0; iBar < PANEL_WIDTH - 1; iBar++) {
            for (int y = 0; y < PANEL_HEIGHT; y++) {
                ledDest = _pTheMapping->XY(iBar, y);
                (*_pTheLeds)[ledDest] = (*_pTheLeds)[_pTheMapping->XY(iBar + 1, y)];
                (*_pTheLeds)[ledDest].subtractFromRGB(4);
            }
        }
        // i ara pintem la nova column
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            (*_pTheLeds)[_pTheMapping->XY(PANEL_WIDTH - 1, y)] = CHSV(_CurrentBaseHue, 255, pTheValues[y]);
        }
        _CurrentBaseHue++;
    }

    // shifts all lines to the top and inserts the new one. The values are interpreted as the intensity (V) in HSV
    // The array is supposed to hold PANEL_WIDTH values
    void PushLine(const uint8_t* pTheValues)
    {
        int ledDest = 0;
        // shift all columns to the left
        for (int iLine = 0; iLine < PANEL_HEIGHT - 1; iLine++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                ledDest = _pTheMapping->XY(x, iLine);
                (*_pTheLeds)[ledDest] = (*_pTheLeds)[_pTheMapping->XY(x, iLine + 1)];
                (*_pTheLeds)[ledDest].subtractFromRGB(7);
            }
        }
        // i ara pintem la nova column
        for (int x = 0; x < PANEL_WIDTH; x++) {
            (*_pTheLeds)[_pTheMapping->XY(x, PANEL_HEIGHT - 1)] = CHSV(_CurrentBaseHue, 255, pTheValues[x]);
        }
        _CurrentBaseHue++;
    }

private:
    CRGBArray<TOTAL_LEDS>* _pTheLeds; // The FastLed object
    IPanelMapping<PANEL_WIDTH, PANEL_HEIGHT>* _pTheMapping; // The class that will provide the mapping coordinates
    Column _TheColumns[PANEL_WIDTH];

    COLOR_SCHEME _ColorScheme;

    uint8_t _CurrentBaseHue;
};

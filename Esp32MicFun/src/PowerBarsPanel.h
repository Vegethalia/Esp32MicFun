#pragma once

#include <FastLED.h>
#include <string>
constexpr uint16_t PANEL_WIDTH_33 = 33; // from 1 to 33
constexpr uint16_t PANEL_WIDTH_64 = 64; // from 1 to 64
constexpr uint16_t PANEL_HEIGHT_8 = 8; // from 1 to 8
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

// Mapping implementation for the 8x64 panel
class PanelMapping64x8 : public IPanelMapping<PANEL_WIDTH_64, PANEL_HEIGHT_8> {
public:
    virtual uint16_t XY(uint16_t x, uint16_t y) override
    {
        // mapping created using the https://macetech.github.io/FastLED-XY-Map-Generator/
        static constexpr uint16_t XYTable[PANEL_WIDTH_64 * PANEL_HEIGHT_8] = {
            504, 503, 488, 487, 472, 471, 456, 455, 440, 439, 424, 423, 408, 407, 392, 391, 376, 375, 360, 359, 344, 343, 328, 327, 312, 311, 296, 295, 280, 279, 264, 263, 248, 247, 232, 231, 216, 215, 200, 199, 184, 183, 168, 167, 152, 151, 136, 135, 120, 119, 104, 103, 88, 87, 72, 71, 56, 55, 40, 39, 24, 23, 8, 7,
            505, 502, 489, 486, 473, 470, 457, 454, 441, 438, 425, 422, 409, 406, 393, 390, 377, 374, 361, 358, 345, 342, 329, 326, 313, 310, 297, 294, 281, 278, 265, 262, 249, 246, 233, 230, 217, 214, 201, 198, 185, 182, 169, 166, 153, 150, 137, 134, 121, 118, 105, 102, 89, 86, 73, 70, 57, 54, 41, 38, 25, 22, 9, 6,
            506, 501, 490, 485, 474, 469, 458, 453, 442, 437, 426, 421, 410, 405, 394, 389, 378, 373, 362, 357, 346, 341, 330, 325, 314, 309, 298, 293, 282, 277, 266, 261, 250, 245, 234, 229, 218, 213, 202, 197, 186, 181, 170, 165, 154, 149, 138, 133, 122, 117, 106, 101, 90, 85, 74, 69, 58, 53, 42, 37, 26, 21, 10, 5,
            507, 500, 491, 484, 475, 468, 459, 452, 443, 436, 427, 420, 411, 404, 395, 388, 379, 372, 363, 356, 347, 340, 331, 324, 315, 308, 299, 292, 283, 276, 267, 260, 251, 244, 235, 228, 219, 212, 203, 196, 187, 180, 171, 164, 155, 148, 139, 132, 123, 116, 107, 100, 91, 84, 75, 68, 59, 52, 43, 36, 27, 20, 11, 4,
            508, 499, 492, 483, 476, 467, 460, 451, 444, 435, 428, 419, 412, 403, 396, 387, 380, 371, 364, 355, 348, 339, 332, 323, 316, 307, 300, 291, 284, 275, 268, 259, 252, 243, 236, 227, 220, 211, 204, 195, 188, 179, 172, 163, 156, 147, 140, 131, 124, 115, 108, 99, 92, 83, 76, 67, 60, 51, 44, 35, 28, 19, 12, 3,
            509, 498, 493, 482, 477, 466, 461, 450, 445, 434, 429, 418, 413, 402, 397, 386, 381, 370, 365, 354, 349, 338, 333, 322, 317, 306, 301, 290, 285, 274, 269, 258, 253, 242, 237, 226, 221, 210, 205, 194, 189, 178, 173, 162, 157, 146, 141, 130, 125, 114, 109, 98, 93, 82, 77, 66, 61, 50, 45, 34, 29, 18, 13, 2,
            510, 497, 494, 481, 478, 465, 462, 449, 446, 433, 430, 417, 414, 401, 398, 385, 382, 369, 366, 353, 350, 337, 334, 321, 318, 305, 302, 289, 286, 273, 270, 257, 254, 241, 238, 225, 222, 209, 206, 193, 190, 177, 174, 161, 158, 145, 142, 129, 126, 113, 110, 97, 94, 81, 78, 65, 62, 49, 46, 33, 30, 17, 14, 1,
            511, 496, 495, 480, 479, 464, 463, 448, 447, 432, 431, 416, 415, 400, 399, 384, 383, 368, 367, 352, 351, 336, 335, 320, 319, 304, 303, 288, 287, 272, 271, 256, 255, 240, 239, 224, 223, 208, 207, 192, 191, 176, 175, 160, 159, 144, 143, 128, 127, 112, 111, 96, 95, 80, 79, 64, 63, 48, 47, 32, 31, 16, 15, 0
        };
        // any out of bounds address maps to the first hidden pixel
        if ((x >= PANEL_WIDTH_64) || (y >= PANEL_HEIGHT_8)) {
            return (PANEL_WIDTH_64 * PANEL_HEIGHT_8) - 1;
        }

        uint16_t i = (y * PANEL_WIDTH_64) + x;
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
    void IncBaseHue(uint8_t step = 1) { _CurrentBaseHue += step; }

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
    // The values inside the "window" ignoreFromX,0,PANEL_WIDTH-1,ignoreToY-1 will be painted in black.
    // To ignore the "window" pass very big X and 0
    void PushLine(const uint8_t* pTheValues, uint16_t ignoreFromX = 10000, uint8_t ignoreToY = 0)
    {
        int ledDest = 0;
        // shift all columns to the top
        for (int iLine = 0; iLine < PANEL_HEIGHT - 1; iLine++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                ledDest = _pTheMapping->XY(x, iLine);
                (*_pTheLeds)[ledDest] = (*_pTheLeds)[_pTheMapping->XY(x, iLine + 1)];
                if (x < ignoreFromX || iLine >= ignoreToY) {
                    (*_pTheLeds)[ledDest].subtractFromRGB(6);
                } else {
                    (*_pTheLeds)[ledDest].subtractFromRGB(192);
                }
            }
        }
        // i ara pintem la nova column
        for (int x = 0; x < PANEL_WIDTH; x++) {
            (*_pTheLeds)[_pTheMapping->XY(x, PANEL_HEIGHT - 1)] = CHSV(_CurrentBaseHue, 255, pTheValues[x]);
        }
        _SubCounter++;
        if (_SubCounter > 4) {
            _CurrentBaseHue++;
            _SubCounter = 0;
        }
    }

    // Draw a portion of the contents of the u8g2 screen buffer in the led panel.
    // The portion drawed goes from pixel 0 to pixel width-1 horizontally, and vertically the number of vertical tiles.
    // The color and value are specified in Hue/Intensity
    void DrawScreenBuffer(const uint8_t* pTheScreenBuffer, uint16_t bufferWidthInTiles, uint16_t width, uint8_t vertTiles, uint8_t hue, uint8_t intensity)
    {
        uint8_t pixColor = _CurrentBaseHue;
        for (uint8_t tileRow = 0; tileRow < vertTiles; tileRow++) {
            for (uint16_t x = 0; x < width; x++) {
                // each byte in the buffer pos represent 8 pixels "vertically", on if the corresponding bit is 1
                // see https://github.com/olikraus/u8g2/wiki/u8g2reference#memory-structure-for-controller-with-u8x8-support
                uint8_t mask = 0x01;
                uint8_t pixelCol = pTheScreenBuffer[x];
                for (uint8_t xbit = 0; xbit < 8; xbit++) {
                    if (pixelCol & mask) { // pintar aquest bit!
                        (*_pTheLeds)[_pTheMapping->XY(x, (tileRow * 8) + xbit)] = CHSV(pixColor, 255, intensity);
                    }
                    mask = mask << 1;
                    pixColor++;
                }
            }
            pTheScreenBuffer += bufferWidthInTiles * 8; // advance to the next tileRow
        }
    }

private:
    CRGBArray<TOTAL_LEDS>* _pTheLeds; // The FastLed object
    IPanelMapping<PANEL_WIDTH, PANEL_HEIGHT>* _pTheMapping; // The class that will provide the mapping coordinates
    Column _TheColumns[PANEL_WIDTH];

    COLOR_SCHEME _ColorScheme;

    uint8_t _CurrentBaseHue;
    uint8_t _SubCounter;
};

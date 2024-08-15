#include "Effects/effect_utils.h"

#include "Effects/Clock.h"

#include "Effects/CalcMode.h"
#include "Effects/CurrentGraph.h"
#include "Effects/DiscoLights.h"
#include "Effects/Frame.h"
#include "Effects/HorizSpectrogram.h"
#include "Effects/InitDemoParametric.h"
#include "Effects/LedBars.h"
#include "Effects/MatrixFFT.h"
#include "Effects/VertSpectrogram.h"
#include "Effects/Wave.h"

//*****************************************************
// Test drawing methods
//*****************************************************

// uint8_t thisSpeed = 3;
// uint8_t initial = 1;
int _delayFrame = 100;

// void DrawParametricHardcoded()
// {
//     static uint16_t currPos = 0;
//     static uint16_t sizePoses = 8; // sizeof(__2to3PiHalf.initialPoints) / sizeof(uint16_t);
//     static HSVHue theHues[8] = { HSVHue::HUE_AQUA, HSVHue::HUE_BLUE, HSVHue::HUE_GREEN, HSVHue::HUE_ORANGE, HSVHue::HUE_PINK, HSVHue::HUE_PURPLE, HSVHue::HUE_RED, HSVHue::HUE_YELLOW };

//     //    FastLED.clear();
//     //_TheLeds.fadeLightBy(90);

//     // for (int i = 0; i < sizePoses; i++) {
//     //     _TheLeds[_TheMapping.XY(
//     //         round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
//     //         round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
//     //         = CHSV(theHues[i], 255, 200);
//     // }
//     for (int i = 0; i < sizePoses; i++) {
//         // float x = __2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
//         // float y = __2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252];
//         // int x1 = (int)x + 1;
//         // int y1 = (int)y + 1;
//         // float x1Percent = x - (int)x;
//         // float x0Percent = 1.0 - x1Percent;
//         // float y1Percent = y - (int)y;
//         // float y0Percent = 1.0 - x1Percent;
//         // float c0Percent = (x0Percent + y0Percent) / 2;
//         // float c1Percent = (x1Percent + y1Percent) / 2;

//         // if (x1 >= _TheMapping.GetWidth() || y1 >= _TheMapping.GetHeight()) {
//         //     _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255);
//         // } else {
//         //     _TheLeds[_TheMapping.XY((int)x, (int)y)] = CHSV(theHues[i], 255, 255 * c0Percent);
//         //     _TheLeds[_TheMapping.XY((int)x1, (int)y1)] = CHSV(theHues[i], 255, 255 * c1Percent);
//         // }

//         _TheLeds[_TheMapping.XY(
//             round(__2to3PiHalf.xCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]),
//             round(__2to3PiHalf.yCoord[(__2to3PiHalf.initialPoints[i] + currPos) % 252]))]
//             = CHSV(theHues[i], 255, 200);
//     }

//     ++currPos;
//     // currPos = currPos % 252;
//     //   FastLED.show();
// }

/*
void DrawImage()
{
    static int pos = 0;
    static int temp = 0;
    for (int i = 0; i < 33; i++) {
        for (int j = 0; j < 16; j++) {
            _TheLeds[_TheMapping.XY((i + pos) % 33, j)] = __imgArletErola2[i][j];
            napplyGamma_video(_TheLeds[_TheMapping.XY((i + pos) % 33, j)], 1.8f);
        }
    }
    pos++;
    // FastLED.show();
}

#include <math.h>
constexpr int pixelCount = 800;
uint8_t sparkHue = HSVHue::HUE_AQUA; // .5; //.2 => fire .1 => redish fire .5 => aqua
int sparkSaturation = 1;
constexpr int numSparks = 1 + (pixelCount / 20);
float decay = 0.995;
uint8_t maxSpeed = 100;
float newThreshhold = 0.03;

float sparks[numSparks];
float sparkX[numSparks];
float pixels[pixelCount];

uint16_t frames = 0;

void DrawSparks()
{
    if (++frames > 500) {
        sparkHue = random8();
        frames = 0;
    }

    float delta = 3; // po ejemplo

    for (int i = 0; i < pixelCount; i++) {
        pixels[i] = pixels[i] * 0.92;
    }

    for (int i = 0; i < numSparks; i++) {
        if (sparks[i] >= -newThreshhold && sparks[i] <= newThreshhold) {
            sparks[i] = ((int)(maxSpeed / 2) - (int)random8(maxSpeed)) / 100.0; // random entre -maxspeed/2 i maxspeed/2, entre 100.0
            sparkX[i] = random16(pixelCount);
        }

        sparks[i] *= decay;
        sparkX[i] += sparks[i] * delta;

        if (sparkX[i] >= pixelCount) {
            sparkX[i] = 0;
        }

        if (sparkX[i] < 0) {
            sparkX[i] = pixelCount - 1;
        }

        pixels[(int)floor(sparkX[i])] += sparks[i]; // com mes "calent" més ràpid
    }

    // log_v("-------------");
    // uint8_t maxv = 0;
    // float maxf = 0.0;
    for (int i = 0; i < pixelCount; i++) {
        float f = (pixels[i] * pixels[i] * 5.0) * 255.0;
        uint8_t v = f > 255.0 ? 255 : (uint8_t)f;
        _TheLeds[i] = CHSV(sparkHue, 255, v);
        // log_v("Pixel[%d]=%d", i, v);
        // if (v > maxv) {
        //     maxv = v;
        // }
        // if (pixels[i] > maxf) {
        //     maxf = pixels[i];
        // }
    }
    // log_v("maxV=%d maxF=%2.3f", maxv, maxf);

    FastLED.show();
    // delay(5);
}
*/
#pragma once

#include "fft.h"
#include <vector>

// Utility class used to get the FFT Power spectrum of an audio signal
class FftPower {
private:
    struct FreqBins {
        uint32_t fromBin;
        uint32_t toBin;
    };

public:
    enum BinResolution {
        ALL = 0, // return all bins (numSamples/2)
        HALF = 1, // combine bins in groups of 2 (numsamples/4)
        QUARTER = 2, // combine bins in groups of 4 (numsamples/8)
        AUTO32 = 3, // return 32 bins "in a intelligent way"
        AUTO34 = 4, // return 33 bins "in a intelligent way"
        AUTO5hz = 5, // return 33 bins "in a intelligent way"
        PIANO33_3Hz = 6, // return 33 bins representing 33 piano keys from hi 2nd octave to low 5th octave
        PIANO64_3Hz = 7, // return 64 bins representing 64 piano keys from full 2nd octave to low 7th octave
        AUTO64_3Hz = 8
    };

    // Constructor. Pass the size of the desired FFT.
    // The fft will be executed once -at least- calculateEverySamples have been inserted since last execution
    FftPower(uint16_t numSamples, uint16_t calculateEverySamples);
    ~FftPower();

    // Return a pointer to the 1st element of the inputArray
    float* GetInputBuffer();

    // Adds a pack of audio samples to the internal circular buffer.
    // Oldest samples are automatically replaced when the internal buffer is filled.
    void AddSamples(const uint16_t* pTheRawSamples, uint16_t numSamples);

    // Executes the FFT plan with the samples present in the internal buffer, if enought samples have been feed since last Execute.
    // If the FFT is performed, returns true, false otherwise (in that case more samples need to be feed).
    // If applyHanning is true, a Hanning window is preapplied before calculating the FFT.
    // Call GetFreqPower after this call to get the filtered results.
    bool Execute(bool applyHanning = true);

    // Call after Execute. Returns the frequency power for each bin. The array must be (numSamples/2)+1 long.
    // values are escaled according to maxFftMagnitude (depends on input levels... 70k is ok)
    // Returns the bin index with the higher power
    void GetFreqPower(int32_t* pFreqPower, uint32_t maxFftMagnitude, BinResolution binRes, uint16_t& maxBin, int32_t& maxMag);

private:
    fft_config_t* _pRealFftPlan;
    // we use 2 sample buffer to make the copy of the new samples + hanning easier
    std::vector<uint16_t> _TheSamplesBuffer;
    std::vector<float> _HanningPrecalc;

    uint16_t _buffPos; // The last position of the _TheSamplesBuffer
    uint16_t _numSamples; // convenient value. Same as _TheSamplesBuffer.size()
    uint16_t _samplesSinceLastExecution; // samples received since last execution
    uint16_t _executeEverySamples; // execute the fft when this number of samples have been received

    const uint8_t _Auto32Groups[32] = {
        2, 3, 4, 5, 6, 7, 9, 11, // 8
        13, 15, 17, 19, 21, 23, 26, 29, // 12
        31, 34, 37, 41, 45, 49, 53, 57, // 24
        61, 65, 73, 81, 89, 97, 105, 128
    }; // 32

    const uint8_t _Auto33Groups[34] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, // 8x1 ~379hz
        10, 12, 14, 16, 18, 20, 22, // 7x2 ~924hz
        26, 30, 34, 38, 42, 46, // 6x4 ~1012Hz
        52, 58, 64, 70, 76, // 5x6 ~3192hz
        83, 90, 97, 104, // 4x7 ~4368Hz
        112, 120, 128 // 3x8 ~5376Hz
    };

    const uint8_t _Auto33Groups_v2[34] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, // 8x1 ~379hz
        10, 12, 14, 16, 18, 20, 22, 24, // 8x2 ~1008hz
        27, 30, 33, 36, 39, 42, 45, 48, // 8x3 ~2016Hz
        53, 58, 63, 68, 73, 78, // 6x5 ~3276hz
        90, 107, 128 // 15-17-21 ~5376hz
    };

    const uint8_t _Auto33Groups_v3[34] = {
        2, 3, 4, 5, 6, 7, 8, 9, // 8x1 ~379hz
        10, 11, 12, 13, 14, 15, 16, 17, 18, // 8x2 ~714hz
        19, 21, 23, 25, 27, 29, 31, 33, // 8x3 ~1386Hz
        36, 40, 45, 51, 58, 66, // 6x5 ~2772hz
        80, 100, 128 // 14-20-28 ~5376hz
    };

    const uint8_t _Auto33Groups_v4[34] = {
        2, 3, 4, 5, 6, 7, 8, 9, 10, // 8x1 ~379hz
        11, 12, 13, 14, 15, 16, 17, 18, // 8x2 ~714hz
        19, 20, 21, 22, 23, 24, 26, 28, // 8x3 ~1386Hz
        32, 34, 37, 40, 45, 55, // 6x5 ~2772hz
        80, 100, 128 // 14-20-28 ~5376hz
    };

    // example from internet, max 17500hz
    // 45, 90, 130, 180, 220, 260, 310, 350, 390, 440, 480, 525, 650, 825, 1000, 1300, 1600, 2050, 2500, 3000, 4000, 5125, 6250, 9125, 12000, 13000, 14000, 15000, 16000, 16500, 17000, 17500

    // 5hz separation. aiming for 40hz, 40x1.18hz....   to get the bin, divide by 5
    //   40,   45,   55,   65,   75,   90,   --> {8, 8} {9, 9} {11, 11} {13,13} {15, 15} {18, 18},
    //  110,  130,  150,  175,  210,  245,   --> {21, 22}, {25, 26}, {29, 31}, {34, 36}, {41, 43}, {48, 50},
    //  290,  345,  405,  480,  565,  665,   --> {57, 59}, {68, 70}, {80, 82}, {94, 98}, {111, 115}, {131, 135},
    //  785,  930, 1095, 1295, 1525, 1800,   --> {155, 159}, {184, 188}, {217, 221}, {256, 262}, {302, 308}, {357, 363},
    // 2125, 2505, 2960, 3490, 4115, 4860,   --> {422, 428}, {498, 604}, {588, 596}, {694, 702}, {819, 827}, {968, 976},
    // 5735, 6770, 7985                      --> {1142, 1152}, {1349, 1359}, {1587, 1607}
    FreqBins _Auto33Bands_v1[33] = {
        { 8, 8 }, { 9, 9 }, { 11, 11 }, { 13, 13 }, { 15, 15 }, { 18, 18 }, //    40,   45,   55,   65,   75,   90,
        { 21, 22 }, { 25, 26 }, { 29, 31 }, { 34, 36 }, { 41, 43 }, { 48, 50 }, // 110,  130,  150,  175,  210,  245,
        { 57, 59 }, { 68, 70 }, { 80, 82 }, { 94, 98 }, { 111, 115 }, { 131, 135 }, // 290,  345,  405,  480,  565,  665
        { 155, 159 }, { 184, 188 }, { 217, 221 }, { 256, 262 }, { 302, 308 }, { 357, 363 }, // 785,  930, 1095, 1295, 1525, 1800
        { 422, 428 }, { 498, 504 }, { 588, 596 }, { 694, 702 }, { 819, 827 }, { 968, 976 }, // 2125, 2505, 2960, 3490, 4115, 4860
        { 1142, 1152 }, { 1349, 1359 }, { 1593, 1603 }
    };

    // 33 piano key frequencies rounded to the closest 3hz freq
    // 104Hz #G2 - 110 A2 - 117 #B2 - 123 B2
    // 131Hz C3 - 139 #C3 - 147 D3 - 156 #D3 - 165 E3 - 175 F3 - 185 #F3 - 196 G3 - 208 #G3 - 220 A3 - 233 #A3 - 247 B3
    // 262Hz C4 - 277 #C4 - 294 D4 - 311 #D4 - 330 E4 - 349 F4 - 370 #F4 - 392 G4 - 415 #G4 - 440 A4 - 466 #A4 - 494 B4
    // 523Hz C5 - 554 #C5 - 587 D5 - 622 #D5 - 659 E5
    FreqBins _33PianoKeys[33] = {
        { 34, 35 }, { 36, 37 }, { 39, 39 }, { 41, 41 }, // 2na octava
        { 43, 44 }, { 46, 46 }, { 49, 49 }, { 52, 52 }, { 55, 55 }, { 58, 58 }, { 61, 62 }, { 65, 65 }, { 69, 69 }, { 73, 73 }, { 77, 78 }, { 82, 82 }, // 3era octava
        { 87, 87 }, { 92, 92 }, { 98, 98 }, { 103, 104 }, { 110, 110 }, { 116, 116 }, { 123, 123 }, { 130, 131 }, { 138, 138 }, { 146, 147 }, { 155, 155 }, { 164, 165 }, // 4arta octava
        { 174, 174 }, { 184, 185 }, { 195, 196 }, { 207, 207 }, { 219, 220 } // 5ena octava
    };

    // 64 piano key frequencies rounded to the closest 3hz freq
    //                                                                                                   55 A1 -          -   62 B1
    //   65Hz C2 -   69 #C2 -   73 D2 -   78 #D2 -   82 E2 -   87 F2 -   93 #F2 -   98 G2 -  104 #G2 -  110 A2 -  117 #B2 -  123 B2
    //  131Hz C3 -  139 #C3 -  147 D3 -  156 #D3 -  165 E3 -  175 F3 -  185 #F3 -  196 G3 -  208 #G3 -  220 A3 -  233 #A3 -  247 B3
    //  262Hz C4 -  277 #C4 -  294 D4 -  311 #D4 -  330 E4 -  349 F4 -  370 #F4 -  392 G4 -  415 #G4 -  440 A4 -  466 #A4 -  494 B4
    //  523Hz C5 -  554 #C5 -  587 D5 -  622 #D5 -  659 E5 -  698 F5 -  740 #F5 -  784 G5 -  831 #G5 -  880 A5 -  932 #A5 -  988 B5
    // 1047Hz C6 - 1109 #C6 - 1175 D6 - 1245 #D6 - 1319 E6 - 1397 F6 - 1480 #F6 - 1568 G6 - 1661 #G6 - 1760 A6 - 1865 #A6 - 1976 B6
    // 2093hZ C7 -          - 2349 D7
    FreqBins _64PianoKeys[64] = {
        { 18, 18 }, { 20, 20 }, // 1era octava
        { 21, 22 }, { 23, 23 }, { 24, 24 }, { 26, 26 }, { 27, 27 }, { 29, 29 }, { 31, 31 }, { 32, 33 }, { 34, 35 }, { 36, 37 }, { 39, 39 }, { 41, 41 }, // 2na octava
        { 43, 44 }, { 46, 46 }, { 49, 49 }, { 52, 52 }, { 55, 55 }, { 58, 58 }, { 61, 62 }, { 65, 65 }, { 69, 69 }, { 73, 73 }, { 77, 78 }, { 82, 82 }, // 3era octava
        { 87, 87 }, { 92, 92 }, { 98, 98 }, { 103, 104 }, { 110, 110 }, { 116, 116 }, { 123, 123 }, { 130, 131 }, { 138, 138 }, { 146, 147 }, { 155, 155 }, { 164, 165 }, // 4arta octava
        { 174, 174 }, { 184, 185 }, { 195, 196 }, { 207, 207 }, { 219, 220 }, { 232, 233 }, { 246, 247 }, { 261, 261 }, { 277, 277 }, { 293, 293 }, { 310, 311 }, { 329, 329 }, // 5ena octava
        { 349, 349 }, { 369, 370 }, { 391, 392 }, { 415, 415 }, { 439, 440 }, { 465, 466 }, { 493, 493 }, { 522, 523 }, { 553, 554 }, { 586, 587 }, { 621, 622 }, { 658, 659 }, // 6ena octava
        { 697, 698 }, { 783, 783 } // 7ena octava
    };

    // 3hz separation. aiming for 54hz, 54x1.078hz....   to get the bin, divide by 3
    //   54,   58,   63,   67,   73,   79,   85,   91,
    //   98,  106,  114,  123,  133,  143,  155,  167,
    //  180,  193,  209,  225,  243,  261,  282,  304,
    //  328,  353,  381,  410,  442,  477,  514,  554,
    //  597,  644,  694,  748,  807,  870,  937, 1010,
    // 1089, 1174, 1265, 1365, 1471, 1586, 1709, 1842,
    // 1987, 2141, 2309, 2489, 2683, 2892, 3118, 3361,
    // 3623, 3905, 4210, 4538, 4892, 5274, 5685, 6129
    FreqBins _Auto64Bands_v1[64] = {
        { 18, 18 }, { 19, 19 }, { 21, 21 }, { 22, 23 }, { 24, 24 }, { 25, 26 }, { 27, 28 }, { 29, 30 },
        { 31, 33 }, { 34, 35 }, { 36, 38 }, { 39, 41 }, { 42, 44 }, { 46, 48 }, { 49, 52 }, { 53, 56 },
        { 57, 60 }, { 62, 65 }, { 67, 70 }, { 72, 75 }, { 78, 81 }, { 84, 87 }, { 91, 94 }, { 98, 101 },
        { 105, 109 }, { 113, 118 }, { 122, 127 }, { 132, 137 }, { 142, 147 }, { 154, 159 }, { 166, 171 }, { 180, 185 },
        { 194, 199 }, { 210, 215 }, { 226, 231 }, { 244, 249 }, { 264, 269 }, { 285, 290 }, { 307, 312 }, { 332, 337 },
        { 358, 363 }, { 386, 391 }, { 417, 422 }, { 451, 455 }, { 485, 490 }, { 524, 529 }, { 565, 570 }, { 609, 614 },
        { 657, 662 }, { 709, 714 }, { 765, 770 }, { 825, 830 }, { 889, 894 }, { 959, 964 }, { 1034, 1039 }, { 1115, 1120 },
        { 1203, 1208 }, { 1297, 1302 }, { 1398, 1403 }, { 1508, 1513 }, { 1626, 1631 }, { 1753, 1758 }, { 1891, 1896 }, { 2040, 2045 }
    };

    // as v1, but only checking indiviual freqs.
    FreqBins _Auto64Bands_v2[64] = {
        { 18, 18 }, { 19, 19 }, { 21, 21 }, { 23, 23 }, { 24, 24 }, { 26, 26 }, { 28, 28 }, { 30, 30 },
        { 33, 33 }, { 35, 35 }, { 38, 38 }, { 41, 41 }, { 44, 44 }, { 48, 48 }, { 52, 52 }, { 56, 56 },
        { 60, 60 }, { 65, 65 }, { 70, 70 }, { 75, 75 }, { 81, 81 }, { 87, 87 }, { 94, 94 }, { 101, 101 },
        { 109, 109 }, { 118, 118 }, { 127, 127 }, { 137, 137 }, { 147, 147 }, { 159, 159 }, { 171, 171 }, { 185, 185 },
        { 199, 199 }, { 215, 215 }, { 231, 231 }, { 249, 249 }, { 269, 269 }, { 290, 290 }, { 312, 312 }, { 337, 337 },
        { 363, 363 }, { 391, 391 }, { 422, 422 }, { 455, 455 }, { 490, 490 }, { 529, 529 }, { 570, 570 }, { 614, 614 },
        { 662, 662 }, { 714, 714 }, { 770, 770 }, { 830, 830 }, { 894, 894 }, { 964, 964 }, { 1039, 1039 }, { 1100, 1100 },
        { 1150, 1150 }, { 1200, 1200 }, { 1300, 1300 }, { 1400, 1400 }, { 1500, 1500 }, { 1650, 1650 }, { 1800, 1800 }, { 2045, 2045 }
    };

    // From internet, 64 bands
    //    45,  90,  130,  180,  220,  260,  310,  350,
    //  390,  440,  480,  525,  565,  610,  650,  690,
    //  735,  780,  820,  875,  920,  950, 1000, 1050,
    // 1080, 1120, 1170, 1210, 1250, 1300, 1340, 1380,
    // 1430, 1470, 1510, 1560, 1616, 1767, 1932, 2113,
    // 2310, 2526, 2762, 3000, 3060, 3120, 3180, 3240,
    // 3300, 3360, 3420, 3480, 3540, 3600, 3660, 3720,
    // 3780, 3840, 3900, 4000, 4100, 4200, 4300, 4400
    // as v1, but only checking indiviual freqs.
    FreqBins _Auto64Bands_v3[64] = {
        { 15, 15 }, { 22, 22 }, { 30, 30 }, { 38, 38 }, { 46, 46 }, { 60, 66 }, { 73, 73 }, { 87, 87 }, { 103, 103 }, { 117, 117 },
        { 130, 130 }, { 147, 147 }, { 160, 160 }, { 175, 175 }, { 188, 188 }, { 203, 203 }, { 217, 217 }, { 230, 230 },
        { 245, 245 }, { 260, 260 }, { 273, 273 }, { 292, 292 }, { 307, 307 }, { 317, 317 }, { 333, 333 }, { 350, 350 },
        { 360, 360 }, { 373, 373 }, { 390, 390 }, { 403, 403 }, { 417, 417 }, { 433, 433 }, { 447, 447 }, { 460, 460 },
        { 476, 478 }, { 489, 491 }, { 502, 504 }, { 519, 521 }, { 538, 540 }, { 588, 590 }, { 643, 645 }, { 703, 705 },
        { 769, 771 }, { 841, 843 }, { 920, 922 }, { 999, 1001 }, { 1019, 1021 }, { 1049, 1051 }, { 1079, 1081 },
        { 1119, 1121 }, { 1139, 1141 }, { 1159, 1161 }, { 1179, 1181 }, { 1199, 1201 }, { 1219, 1221 }, { 1239, 1241 },
        { 1259, 1261 }, { 1279, 1281 }, { 1299, 1301 }, { 1332, 1334 }, { 1366, 1368 }, { 1399, 1401 }, { 1432, 1434 }, { 1466, 1468 }
    };
};

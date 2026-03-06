#include "Arduino.h"

// include globals
#include "GlobalDefines.h"

// this one after Arduino.h to avoid some errors
#include "FftPower.h"

float q_rsqrt(float number) {
  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y = number;
  i = *(long*)&y;             // evil floating point bit level hacking
  i = 0x5f3759df - (i >> 1);  // what the fuck?
  y = *(float*)&i;
  y = y * (threehalfs - (x2 * y * y));  // 1st iteration
  // y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

  return y;
}

namespace {
constexpr float MIN_MAGNITUDE_RATIO = 1e-12f;

inline float MagnitudeFromSquared(float squaredMagnitude) {
  if (squaredMagnitude <= 0.0f) {
    return 0.0f;
  }
  return 1.0f / q_rsqrt(squaredMagnitude);
}

inline float MagnitudeFromComplex(float real, float imag) {
  return MagnitudeFromSquared((real * real) + (imag * imag));
}

inline int8_t MagnitudeToDb(float magnitude, uint32_t maxFftMagnitude) {
  float referenceMagnitude = maxFftMagnitude > 0 ? (float)maxFftMagnitude : 1.0f;
  if (magnitude > referenceMagnitude) {
    magnitude = referenceMagnitude;
  }
  float ratio = magnitude / referenceMagnitude;
  if (ratio < MIN_MAGNITUDE_RATIO) {
    ratio = MIN_MAGNITUDE_RATIO;
  }
  return (int8_t)(20.0f * log10f(ratio));
}
}  // namespace

FftPower::FftPower(uint16_t numSamples, uint16_t calculateEverySamples) {
  _buffPos = 0;
  _samplesSinceLastExecution = 0;
  _executeEverySamples = calculateEverySamples;
  _numSamples = numSamples;
  _TheSamplesBuffer.resize(numSamples);

  // int fftSize = max(numSamples, DEFAULT_FFTSIZE);
  _pRealFftPlan = fft_init(numSamples, FFT_REAL, FFT_FORWARD, nullptr, nullptr);

  _HanningPrecalc.resize(numSamples);

  // Blackman–Harris a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168
  // blackman-nuttall a0 = 0.3635819, a1 = 0.4891775, a2 = 0.1365995, a3 = 0.0106411
  // nutall a0=0.355768; a1=0.487396; a2=0.144232 a3=0.012604.
  // const double a0 = 0.3635819,
  //              a1 = 0.4891775,
  //              a2 = 0.1365995,
  //              a3 = 0.0106411,
  //              f = (2.0f * PI) / (float)(numSamples - 1);

  // const double a0 = 0.35875,
  //              a1 = 0.48829,
  //              a2 = 0.14128,
  //              a3 = 0.01168;
  // const double f = (2.0f * PI) / (float)(numSamples);

  // for (uint16_t i = 0; i < numSamples; i++) {
  //     float nutt = a0 - a1 * cos((float)i * f) + a2 * cos(2 * (float)i * f) - a3 * cos(3 * (float)i * f);
  //     _HanningPrecalc[i] = nutt;
  // }

  // HANN WINDOW
  const double f = (2.0f * PI) / (float)(numSamples - 1);
  for (uint16_t i = 0; i < numSamples; i++) {
    // apply hann window w[n]=0.5·(1-cos(2Pi·n/N))=sin^2(Pi·n/N)
    // auto hann = 0.5f * (1 - cos((2.0f * PI * (float)k) / (float)(AUDIO_DATA_OUT)));
    float hann = 0.5f * (1.0f - cos(((float)i * f)));
    _HanningPrecalc[i] = hann;
    //     *0.25 * sin((2.0f * PI * (float)i) / (float)(AUDIO_DATA_OUT - 1));
  }
}

FftPower::~FftPower() {
  fft_destroy(_pRealFftPlan);
  _pRealFftPlan = nullptr;
}

// Return a pointer to the 1st element of the inputArray
float* FftPower::GetInputBuffer() {
  return _pRealFftPlan ? _pRealFftPlan->input : nullptr;
}

// Adds a pack of audio samples to the internal circular buffer.
// Oldest samples are automatically replaced when the internal buffer is filled.
void FftPower::AddSamples(const int16_t* pTheRawSamples, uint16_t numSamples) {
  // cap tot?
  if (_buffPos + numSamples < _numSamples) {
    //  log_d("FullCopy Adding [%d] samples at pos [%d]", numSamples, _buffPos);
    memcpy(_TheSamplesBuffer.data() + _buffPos, pTheRawSamples, numSamples * sizeof(int16_t));
    _buffPos += numSamples;
  } else {  // no hi cap tot, primer copiem al final del buffer i despres el que queda al principi
    uint16_t endBuffSamples = min((int)_numSamples - (int)_buffPos, (int)numSamples);
    uint16_t startBuffSamples = numSamples - endBuffSamples;
    memcpy(_TheSamplesBuffer.data() + _buffPos, pTheRawSamples, endBuffSamples * sizeof(int16_t));
    //  log_d("PartialCopy Adding [%d] samples at pos [%d].", endBuffSamples, _buffPos);
    if (startBuffSamples) {
      log_d("And PartialCopy Adding [%d] samples at pos 0. ", startBuffSamples);
      memcpy(_TheSamplesBuffer.data(), pTheRawSamples + endBuffSamples, startBuffSamples * sizeof(int16_t));
    }
    _buffPos = (_buffPos + numSamples) % _numSamples;
    // log_d("New pos [%d]", _buffPos);
  }
  _samplesSinceLastExecution += numSamples;
}

// Executes the FFT plan. Call after setting numsamples of data GetInputBuffer().
// Call GetFreqPower after this call
bool FftPower::Execute(bool applyHanning, uint16_t zeroValue) {
  if (_samplesSinceLastExecution < _executeEverySamples) {
    // log_d("Execute Called. Not enought new samples: [%d/%d]. ", _samplesSinceLastExecution, _executeEverySamples);

    return false;
  }
  uint16_t pos = 0;

  if (applyHanning) {
    for (uint16_t i = 0; i < _numSamples; i++) {
      pos = (_buffPos + i) % _numSamples;
      _pRealFftPlan->input[i] = _HanningPrecalc[i] * (float)_TheSamplesBuffer[pos] - (float)zeroValue;
    }
  } else {
    for (uint16_t i = 0; i < _numSamples; i++) {
      pos = (_buffPos + i) % _numSamples;
      _pRealFftPlan->input[i] = (float)_TheSamplesBuffer[pos] - (float)zeroValue;
    }
  }
  fft_execute(_pRealFftPlan);

  // log_d("Execute Called with enought samples: [%d/%d]. ", _samplesSinceLastExecution, _executeEverySamples);
  _samplesSinceLastExecution = 0;

  return true;
}

// Call after Execute. Returns the frequency power for each bin. The array must be (numSamples/2)+1 long.
// values are escaled according to maxFftMagnitude (depends on input levels... 70k is ok)
// Returns the bin index with the higher power
// numFreqsOut => size of pFreqPower array, or max number of values to return
void FftPower::GetFreqPower(int8_t* pFreqPower, uint16_t numFreqsOut, uint32_t maxFftMagnitude, BinResolution binRes, uint16_t& maxBin, int32_t& maxMag) {
  uint16_t maxIndex = _TheSamplesBuffer.size() / 2;

  // log_d("GetFreqPower %d", binRes);

  maxBin = 0;
  maxMag = -10000;
  pFreqPower[0] = -100;  // dc is not calculated

  if (binRes == FftPower::AUTO34) {  // binRes==BinResolution::AUTO32
    uint16_t ind = 1;
    const uint8_t* pBinIndexes = _Auto33Groups_v4;  //_Auto32Groups;
    maxIndex = 34;
    // else {
    //     maxIndex = 32;
    //     pBinIndexes = _Auto32Groups;
    // }

    do {
      float bandMaxMagnitude = 0.0f;
      // log_d("Bucle %d", ind);

      for (uint16_t k = pBinIndexes[ind - 1]; k < pBinIndexes[ind]; k++) {
        const uint16_t fftOutIndex = k * 2;
        float mag = MagnitudeFromComplex(_pRealFftPlan->output[fftOutIndex], _pRealFftPlan->output[fftOutIndex + 1]);
        if (mag > bandMaxMagnitude) {
          bandMaxMagnitude = mag;
        }
        // log_d("       Bucle %d - %d", ind,k);
      }
      pFreqPower[ind] = MagnitudeToDb(bandMaxMagnitude, maxFftMagnitude);

      if ((int32_t)bandMaxMagnitude > maxMag) {
        maxMag = (int32_t)bandMaxMagnitude;
        maxBin = pBinIndexes[ind - 1];
      }
      ind++;
    } while (ind < maxIndex);
  } else if (binRes == FftPower::MATRIX) {
    int16_t maxComplexBins = (int16_t)((_numSamples / 2) - 1);
    int16_t numBins = min((int16_t)numFreqsOut, maxComplexBins);
    if (numBins <= 0) {
      return;
    }
    int16_t currentBin = 1;
    int16_t everyBin = max((int16_t)1, (int16_t)(maxComplexBins / numBins));

    for (uint32_t ind = 0; ind < numBins; ind++, currentBin += everyBin) {
      if (currentBin > maxComplexBins) {
        currentBin = maxComplexBins;
      }
      //  log_d("FromBin [%04d] ToBin [%02d]", fromBin, toBin);
      const uint16_t fftOutIndex = currentBin * 2;
      float mag = MagnitudeFromComplex(_pRealFftPlan->output[fftOutIndex], _pRealFftPlan->output[fftOutIndex + 1]);
      pFreqPower[ind] = MagnitudeToDb(mag, maxFftMagnitude);

      if (mag > maxMag) {
        maxMag = (int32_t)mag;
        maxBin = ind;
      }
      // if (ind < 10) {
      //     log_d("Bin [%02d] - %02d", ind, pFreqPower[ind]);
      // }
      // if (ind < 200) {
      //     log_d("Bin [%02d] - Mag=%6.2f Dbs=%02d", ind, mag, pFreqPower[ind]);
      // }
    }
  } else if (binRes == FftPower::PIANO64_6Hz) {
    const uint16_t* pBinsNew = _64PianoKeys_v2_6Hz;
    uint8_t numBins = 64;
    maxBin = 0;
    uint32_t theBin = 0;
    for (uint16_t ind = 0; ind < numBins; ind++) {
      theBin = (pBinsNew[ind]) * 2;  //*2 pq _pRealFftPlan->output contains real,img parts interleaved.
      float mag = MagnitudeFromComplex(_pRealFftPlan->output[theBin], _pRealFftPlan->output[theBin + 1]);
      pFreqPower[ind] = MagnitudeToDb(mag, maxFftMagnitude);  // power en db's del bin indicat.

      if (mag > maxMag) {
        maxMag = (int32_t)mag;
        maxBin = ind;
      }
    }
  } else if (binRes == FftPower::PIANO48_6Hz) {
    const uint16_t* pBinsNew = _48PianoKeys_v1_6Hz;
    uint8_t numBins = 48;
    maxBin = 0;
    uint32_t theBin = 0;
    uint8_t initialBin = 0;
#if defined(PANEL_SIZE_64x32)
    initialBin = 8;  // we leave the first 8 bins and the last 8 empty. We just show the 48 central bins
    for (uint8_t ind = 0; ind < initialBin; ind++) {
      pFreqPower[ind] = -100;
      pFreqPower[64 - ind - 1] = -100;
    }
#endif
    for (uint16_t ind = 0; ind < numBins; ind++) {
      theBin = (pBinsNew[ind]) * 2;  //*2 pq _pRealFftPlan->output contains real,img parts interleaved.
      float mag = MagnitudeFromComplex(_pRealFftPlan->output[theBin], _pRealFftPlan->output[theBin + 1]);
#if defined(PANEL_SIZE_96x54)
      pFreqPower[(ind * 2)] = MagnitudeToDb(mag, maxFftMagnitude);
      pFreqPower[(ind * 2) + 1] = pFreqPower[(ind * 2)];
#elif defined(PANEL_SIZE_64x32)
      pFreqPower[ind + initialBin] = MagnitudeToDb(mag, maxFftMagnitude);
#endif
      if (mag > maxMag) {
        maxMag = (int32_t)mag;
        maxBin = pBinsNew[ind];
      }
    }
  } else if (binRes == FftPower::PIANO96_3Hz || binRes == FftPower::AUTO96_3Hz || binRes == FftPower::AUTO96_6Hz) {
    const uint16_t* pBinsNew = _96PianoKeys_v1_3Hz;
    uint8_t numBins = 96;
    maxBin = 0;
    uint32_t theBin = 0;
    if (binRes == FftPower::AUTO96_3Hz) {
      pBinsNew = _Auto96Bands_v1_3Hz;
    } else if (binRes == FftPower::AUTO96_6Hz) {
      pBinsNew = _Auto96Bands_v1_6Hz;
    }

    for (uint16_t ind = 0; ind < numBins; ind++) {
      theBin = (pBinsNew[ind]) * 2;  //*2 pq _pRealFftPlan->output contains real,img parts interleaved.
      float mag = MagnitudeFromComplex(_pRealFftPlan->output[theBin], _pRealFftPlan->output[theBin + 1]);
      pFreqPower[ind] = MagnitudeToDb(mag, maxFftMagnitude);  // power en db's del bin indicat.

      if (mag > maxMag) {
        maxMag = (int32_t)mag;
        maxBin = pBinsNew[ind];
      }
    }
  } else if (binRes == FftPower::AUTO64_6Hz) {
    const uint16_t* pBinsNew = _Auto64Bands_v4_6Hz;  //_Auto64Bands_v4_6Hz;
    uint8_t numBins = 64;
    maxBin = 0;
    uint32_t fromBin = 4 * 2;  //*2 pq _pRealFftPlan->output contains real,img parts interleaved. ENs saltem els 4 1ers bins (low precission on mic).
    uint32_t toBin = 0;
    for (uint16_t ind = 0; ind < numBins; ind++) {
      if (ind > 0) {
        fromBin = (pBinsNew[ind - 1]) * 2;
      }
      toBin = (pBinsNew[ind]) * 2;  //*2 pq _pRealFftPlan->output contains real,img parts interleaved

      float sumBin = 0;
      for (uint16_t subInd = fromBin; subInd < toBin; subInd += 2) {
        float auxSum = (_pRealFftPlan->output[subInd] * _pRealFftPlan->output[subInd]) + (_pRealFftPlan->output[subInd + 1] * _pRealFftPlan->output[subInd + 1]);
        // auxSum = (int32_t)(10.0 * log((float)sqrt(auxSum) / (float)maxFftMagnitude));

        if (auxSum > sumBin) {
          sumBin = auxSum;
        }
        // sumBin += auxSum;
      }
      // sumBin = (int32_t)(10.0 * log((float)sqrt(sumBin) / (float)maxFftMagnitude)); // max energy of all bins
      //  sumBin = (int32_t)(10.0 * log((float)sqrt(sumBin / numBins) / (float)maxFftMagnitude));
      //  sumBin = (int32_t)(10.0 * log(1.0f / (q_rsqrt(sumBin / numBins) * (float)maxFftMagnitude))); //avg energy of all bins
      float mag = MagnitudeFromSquared(sumBin);
      pFreqPower[ind] = MagnitudeToDb(mag, maxFftMagnitude);  // màxima energia del rang en dB.

      if (mag > maxMag) {
        maxMag = (int32_t)mag;
        maxBin = (fromBin + 2) / 2;
      }
      // if(ind<10) {
      //     log_d("Bin [%02d] - %02d", ind, pFreqPower[ind]);
      // }
      fromBin = toBin;
    }
  }
}

#ifdef UNIT_TEST
void FftPower::ClearOutputForTest() {
  if (!_pRealFftPlan) {
    return;
  }
  for (uint16_t i = 0; i < _numSamples; i++) {
    _pRealFftPlan->output[i] = 0.0f;
  }
}

void FftPower::SetOutputBinForTest(uint16_t bin, float real, float imag) {
  if (!_pRealFftPlan) {
    return;
  }
  uint16_t outIndex = bin * 2;
  if (outIndex + 1 >= _numSamples) {
    return;
  }
  _pRealFftPlan->output[outIndex] = real;
  _pRealFftPlan->output[outIndex + 1] = imag;
}
#endif
// void NewAudio(uint16_t *pAudioIn, uint16_t samplesIn)
// {

// }

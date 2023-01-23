#include "Arduino.h"

#include "FftPower.h"

FftPower::FftPower(uint16_t numSamples, uint16_t calculateEverySamples)
{

    _buffPos = 0;
    _samplesSinceLastExecution = 0;
    _executeEverySamples = calculateEverySamples;
    _numSamples = numSamples;
    _TheSamplesBuffer.resize(numSamples);

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

    const double a0 = 0.355768,
                 a1 = 0.487396,
                 a2 = 0.144232,
                 a3 = 0.012604,
                 // f = (2.0f * PI) / (float)(numSamples - 1);
        f = (2.0f * PI) / (float)(numSamples);

    for (uint16_t i = 0; i < numSamples; i++) {
        float nutt = a0 - a1 * cos((float)i * f) + a2 * cos(2 * (float)i * f) - a3 * cos(3 * (float)i * f);
        _HanningPrecalc[i] = nutt;
    }

    // HANN WINDOW
    // for (uint16_t i = 0; i < numSamples; i++) {
    //     // apply hann window w[n]=0.5·(1-cos(2Pi·n/N))=sin^2(Pi·n/N)
    //     // auto hann = 0.5f * (1 - cos((2.0f * PI * (float)k) / (float)(AUDIO_DATA_OUT)));
    //     float hann = 0.5f * (1.0f - cos((2.0f * PI * (float)i) / (float)(numSamples - 1)));
    //     _HanningPrecalc[i] = hann;
    //     //     *0.25 * sin((2.0f * PI * (float)i) / (float)(AUDIO_DATA_OUT - 1));
    // }
}

FftPower::~FftPower()
{
    fft_destroy(_pRealFftPlan);
    _pRealFftPlan = nullptr;
}

// Return a pointer to the 1st element of the inputArray
float* FftPower::GetInputBuffer()
{
    return _pRealFftPlan ? _pRealFftPlan->input : nullptr;
}

// Adds a pack of audio samples to the internal circular buffer.
// Oldest samples are automatically replaced when the internal buffer is filled.
void FftPower::AddSamples(const uint16_t* pTheRawSamples, uint16_t numSamples)
{
    // std::vector<uint16_t>* pTheBuffer = nullptr;
    // std::vector<uint16_t>* pTheOtherBuffer = nullptr;

    // if (_bufferInUse == 1) {
    //     pTheBuffer = &_TheSamplesBuffer1;
    //     pTheOtherBuffer = &_TheSamplesBuffer2;
    // } else {
    //     pTheBuffer = &_TheSamplesBuffer2;
    //     pTheOtherBuffer = &_TheSamplesBuffer1;
    // }

    // // Initial loading of the samples buffer
    // if (_buffPos < (pTheBuffer->size() - numSamples)) {
    //     log_d("InitialLoad Adding [%d] samples at pos [%d]", numSamples, _buffPos);
    //     memcpy(pTheBuffer->data(), pTheRawSamples + _buffPos, numSamples * sizeof(uint16_t));
    //     _buffPos += numSamples;
    // } else { // once the buffer is full, use the other to avoid working with a circular buffer and copy the samples
    //     log_d("NextLoad. Buffer=[%d]. Adding [%d] samples at pos [%d]", _bufferInUse, numSamples, (pTheOtherBuffer->size() - numSamples));
    //     memcpy(pTheOtherBuffer, pTheBuffer + numSamples, pTheBuffer->size() - numSamples);
    //     memcpy(pTheOtherBuffer + (pTheOtherBuffer->size() - numSamples), pTheRawSamples, numSamples);
    //     _bufferInUse = _bufferInUse == 1 ? 2 : 1;
    // }

    // cap tot?
    if (_buffPos + numSamples < _numSamples) {
        //  log_d("FullCopy Adding [%d] samples at pos [%d]", numSamples, _buffPos);
        memcpy(_TheSamplesBuffer.data() + _buffPos, pTheRawSamples, numSamples * sizeof(uint16_t));
        _buffPos += numSamples;
    } else { // no hi cap tot, primer copiem al final del buffer i despres el que queda al principi
        uint16_t endBuffSamples = min((int)_numSamples - (int)_buffPos, (int)numSamples);
        uint16_t startBuffSamples = numSamples - endBuffSamples;
        memcpy(_TheSamplesBuffer.data() + _buffPos, pTheRawSamples, endBuffSamples * sizeof(uint16_t));
        //  log_d("PartialCopy Adding [%d] samples at pos [%d].", endBuffSamples, _buffPos);
        if (startBuffSamples) {
            log_d("And PartialCopy Adding [%d] samples at pos 0. ", startBuffSamples);
            memcpy(_TheSamplesBuffer.data(), pTheRawSamples + endBuffSamples, startBuffSamples * sizeof(uint16_t));
        }
        _buffPos = (_buffPos + numSamples) % _numSamples;
        // log_d("New pos [%d]", _buffPos);
    }
    _samplesSinceLastExecution += numSamples;
}

// Executes the FFT plan. Call after setting numsamples of data GetInputBuffer().
// Call GetFreqPower after this call
bool FftPower::Execute(bool applyHanning)
{
    if (_samplesSinceLastExecution < _executeEverySamples) {
        // log_d("Execute Called. Not enought new samples: [%d/%d]. ", _samplesSinceLastExecution, _executeEverySamples);

        return false;
    }
    uint16_t pos = 0;

    if (applyHanning) {
        for (uint16_t i = 0; i < _numSamples; i++) {
            pos = (_buffPos + i) % _numSamples;
            _pRealFftPlan->input[i] = _HanningPrecalc[i] * (float)_TheSamplesBuffer[pos];
        }
    } else {
        for (uint16_t i = 0; i < _numSamples; i++) {
            pos = (_buffPos + i) % _numSamples;
            _pRealFftPlan->input[i] = (float)_TheSamplesBuffer[pos];
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
void FftPower::GetFreqPower(int32_t* pFreqPower, uint32_t maxFftMagnitude, BinResolution binRes, uint16_t& maxBin, int32_t& maxMag)
{
    uint16_t maxIndex = _TheSamplesBuffer.size() / 2;

    // log_d("GetFreqPower %d", binRes);

    maxBin = 0;
    maxMag = -10000;
    pFreqPower[0] = -100; // dc is not calculated

    if (binRes < BinResolution::AUTO32) {
        uint8_t avgN = 1;
        switch (binRes) {
        case BinResolution::ALL:
            avgN = 1;
            break;
        case BinResolution::HALF:
            avgN = 2;
            break;
        case BinResolution::QUARTER:
            avgN = 4;
            break;
        }
        for (uint16_t i = 0, ind = 0; i <= (maxIndex - avgN); i += avgN, ind++) {
            pFreqPower[ind] = -100;
            for (uint8_t k = i; k < i + avgN; k++) {
                if (k > 0) {
                    auto value = (int32_t)sqrt(pow(_pRealFftPlan->output[k * 2], 2) + pow(_pRealFftPlan->output[(k * 2) + 1], 2));
                    if (value > pFreqPower[ind]) {
                        pFreqPower[ind] = value;
                    }
                }
            }
            // pFreqPower[ind] = pFreqPower[ind]/avgN;
            pFreqPower[ind] = (10.0 * log((float)pFreqPower[ind] / (float)maxFftMagnitude));
            // log_d("%d", _TaskParams.fftMag[i]);
            if (pFreqPower[ind] > maxMag) {
                maxMag = pFreqPower[ind];
                maxBin = i;
            }
        }
    } else if (binRes == BinResolution::AUTO34) { // binRes==BinResolution::AUTO32
        uint16_t ind = 1;
        int32_t mag = 0;
        const uint8_t* pBinIndexes = _Auto32Groups;
        uint8_t currInd = 0, upToInd = 0;

        if (binRes == BinResolution::AUTO34) {
            maxIndex = 34;
            pBinIndexes = _Auto33Groups_v4;
        } else {
            maxIndex = 32;
            pBinIndexes = _Auto32Groups;
        }
        currInd = pBinIndexes[0];
        do {
            pFreqPower[0] = 0;
            // log_d("Bucle %d", ind);
            // upToInd = pBinIndexes[currInd];
            // do{

            // }while(currInd<);

            for (uint16_t k = pBinIndexes[ind - 1]; k < pBinIndexes[ind]; k++) {
                mag = (int32_t)sqrt(pow(_pRealFftPlan->output[k * 2], 2) + pow(_pRealFftPlan->output[(k * 2) + 1], 2));
                if (mag > pFreqPower[ind]) {
                    pFreqPower[ind] = mag;
                }
                // log_d("       Bucle %d - %d", ind,k);
            }
            // log_d("ind=%d avg=%d", ind, avg);
            pFreqPower[ind] = (10.0 * log((float)pFreqPower[ind] / (float)maxFftMagnitude));
            // log_d("%d - %d", ind, pFreqPower[ind]);

            if (pFreqPower[ind] > maxMag) {
                maxMag = pFreqPower[ind];
                maxBin = _Auto32Groups[ind - 1];
            }
            ind++;
        } while (ind < maxIndex);
    } else if (binRes == BinResolution::AUTO5hz || binRes == BinResolution::AUTO64_3Hz || binRes == BinResolution::PIANO33_3Hz || binRes == BinResolution::PIANO64_3Hz) { // BinResolution::AUTO5hz
        const FreqBins* pBins = nullptr;
        uint8_t numBins = 32;
        if (binRes == BinResolution::AUTO5hz) {
            pBins = _Auto33Bands_v1;
            numBins = 33;
        } else if (binRes == BinResolution::PIANO33_3Hz) {
            pBins = _33PianoKeys;
            numBins = 33;
            // maxFftMagnitude += 50000;
        } else if (binRes == BinResolution::PIANO64_3Hz) {
            pBins = _64PianoKeys;
            numBins = 64;
        } else if (binRes == BinResolution::AUTO64_3Hz) {
            pBins = _Auto64Bands_v3;
            numBins = 64;
        }

        // log_d("-------------------------------------------------");
        //  1
        //   auto maxMagn = maxFftMagnitude;
        //   for (int i = 50; i < 100; i++) {
        //       auto fromBin = i * 2;
        //       auto value = (int32_t)sqrt(pow(_pRealFftPlan->output[fromBin], 2) + pow(_pRealFftPlan->output[fromBin + 1], 2));
        //       if (value > maxMagn) {
        //           log_d("MAX_MAGNITUDE=%d", value);
        //           maxMagn = value;
        //       }
        //       int valueDb = (int)(10.0 * log((float)value / (float)maxFftMagnitude));
        //       log_d("Bin [%03d - %3.2fHz]=[%05d]", i, (i+0) * 2.5, valueDb);
        //   }
        //  2
        //  auto maxMagn = 0;
        //  int maxI = 0;
        //  for (int i = 5; i < 1024; i++) {
        //      auto fromBin = i * 2;
        //      auto value = (int32_t)sqrt(pow(_pRealFftPlan->output[fromBin], 2) + pow(_pRealFftPlan->output[fromBin + 1], 2));
        //      if (value > maxMagn) {
        //          //log_d("MAX_MAGNITUDE=%d", value);
        //          maxMagn = value;
        //          maxI = i;
        //      }
        //  }
        //  int valueDb = (int)(10.0 * log((float)maxMagn / (float)maxFftMagnitude));
        //  //log_d("Bin [%04d - %4.1fHz]=[%02d]", maxI, (maxI + 0) * 2.5005, valueDb);
        //  log_d("Bin [%04d - %4.1fHz]=[%02d]", maxI, (maxI + 0) * 5.005, valueDb);

        int maxBinPow = -100;
        maxBin = 0;
        for (uint32_t ind = 0; ind < numBins; ind++) {
            uint32_t fromBin = pBins[ind].fromBin * 2; //*2 pq _pRealFftPlan->output contains real,img parts interleaved
            uint32_t toBin = pBins[ind].toBin * 2;
            pFreqPower[ind] = -100; //       (int32_t) sqrt(pow(_pRealFftPlan->output[fromBin], 2) + pow(_pRealFftPlan->output[fromBin + 1], 2));
            //  log_d("FromBin [%04d] ToBin [%02d]", fromBin, toBin);

            for (uint16_t subBin = fromBin; subBin <= toBin; subBin += 2) {
                //  log_d("subBin [%04d]", subBin);
                // auto aux = (int32_t)sqrt(pow(_pRealFftPlan->output[subBin], 2) + pow(_pRealFftPlan->output[subBin + 1], 2));
                auto aux = (int32_t)(pow(_pRealFftPlan->output[subBin], 2) + pow(_pRealFftPlan->output[subBin + 1], 2));
                if (aux > pFreqPower[ind]) {
                    pFreqPower[ind] = aux;
                }
            }
            pFreqPower[ind] = (int32_t)(10.0 * log((float)sqrt(pFreqPower[ind]) / (float)maxFftMagnitude));

            if (pFreqPower[ind] > maxBinPow) {
                maxBinPow = pFreqPower[ind];
                maxBin = ind;
            }
            // if(ind<10) {
            //     log_d("Bin [%02d] - %02d", ind, pFreqPower[ind]);
            // }
        }
        // log_d("MaxBin %02d - Pow %02d - Freq %4.2fHz", maxBin, pFreqPower[maxBin], (pBins[maxBin].fromBin + pBins[maxBin].toBin) / 2.0 * 3.0);
    }
}
// void NewAudio(uint16_t *pAudioIn, uint16_t samplesIn)
// {

// }

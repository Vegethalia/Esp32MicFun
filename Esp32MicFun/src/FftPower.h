#pragma once

#include "fft.h"

//Utility class used to get the FFT Power spectrum of an audio signal
class FftPower
{
public:
	enum BinResolution {
		ALL=0,     //return all bins (numSamples/2)
		HALF=1,    //combine bins in groups of 2 (numsamples/4)
		QUARTER=2, //combine bins in groups of 4 (numsamples/8)
		AUTO32=3   //return 32 bins "in a intelligent way"
	};

	//Constructor. Pass the size of the input audio samples buffer
	FftPower(uint16_t numSamples)
	{
		_numSamples=numSamples;
		_pRealFftPlan = fft_init(numSamples, FFT_REAL, FFT_FORWARD, nullptr, nullptr);
	}

	~FftPower()
	{
		fft_destroy(_pRealFftPlan);
		_pRealFftPlan=nullptr;
	}

	//Return a pointer to the 1st element of the inputArray
	float* GetInputBuffer()
	{
		return _pRealFftPlan ? _pRealFftPlan->input : nullptr;
	}

	//Executes the FFT plan. Call after setting numsamples of data GetInputBuffer().
	//Call GetFreqPower after this call
	void Execute()
	{
		fft_execute(_pRealFftPlan);
	}

	//Call after Execute. Returns the frequency power for each bin. The aary must be (numSamples/2)+1 long.
	//values are escaled according to maxFftMagnitude (depends on input levels... 70k is ok)
	//Returns the bin index with the higher power
	void GetFreqPower(int32_t* pFreqPower, uint32_t maxFftMagnitude, BinResolution binRes, uint16_t& maxBin)
	{
		int32_t maxMag=-10000;
		uint16_t maxIndex=_numSamples / 2;
		maxBin=0;
		pFreqPower[0]=0; //dc is not calculated

		if(binRes!=BinResolution::AUTO32) {
			uint8_t avgN=1;
			switch(binRes) {
				case BinResolution::ALL:
					avgN=1;
					break;
				case BinResolution::HALF:
					avgN=2;
					break;
				case BinResolution::QUARTER:
					avgN=4;
					break;
			}
			for(uint16_t i = 1, ind=1; i < maxIndex; i+=avgN, ind++) {
				pFreqPower[ind]=0;
				for(uint8_t k=i; k<i+avgN; k++) {
					pFreqPower[ind] += (int32_t)sqrt(pow(_pRealFftPlan->output[k * 2], 2) + pow(_pRealFftPlan->output[(k * 2) + 1], 2));
				}
				pFreqPower[ind] = pFreqPower[ind]/avgN;
				pFreqPower[ind] = (10.0 * log((float)pFreqPower[ind] / (float)maxFftMagnitude));
				//log_d("%d", _TaskParams.fftMag[i]);
				if(pFreqPower[ind] > maxMag) {
					maxMag = pFreqPower[ind];
					maxBin=i;
				}
			}
		}
	}
	// void NewAudio(uint16_t *pAudioIn, uint16_t samplesIn)
	// {

	// }

private:
					// Create the FFT config structure
	fft_config_t* _pRealFftPlan;
	uint16_t _numSamples;

};

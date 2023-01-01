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
		AUTO32=3,  //return 32 bins "in a intelligent way"
		AUTO34=4   //return 33 bins "in a intelligent way"
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

	//Call after Execute. Returns the frequency power for each bin. The array must be (numSamples/2)+1 long.
	//values are escaled according to maxFftMagnitude (depends on input levels... 70k is ok)
	//Returns the bin index with the higher power
	void GetFreqPower(int32_t* pFreqPower, uint32_t maxFftMagnitude, BinResolution binRes, uint16_t& maxBin, int32_t &maxMag)
	{
		uint16_t maxIndex = _numSamples / 2;

		maxBin=0;
		maxMag = -10000;
		pFreqPower[0]=-100; //dc is not calculated

		if(binRes<BinResolution::AUTO32) {
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
			for(uint16_t i = 0, ind=0; i <= (maxIndex-avgN); i+=avgN, ind++) {
				pFreqPower[ind]=-100;
				for(uint8_t k=i; k<i+avgN; k++) {
					if(k>0) {
						auto value = (int32_t)sqrt(pow(_pRealFftPlan->output[k * 2], 2) + pow(_pRealFftPlan->output[(k * 2) + 1], 2));
						if(value>pFreqPower[ind]) {
							pFreqPower[ind] = value;
						}
					}
				}
				// pFreqPower[ind] = pFreqPower[ind]/avgN;
				pFreqPower[ind] = (10.0 * log((float)pFreqPower[ind] / (float)maxFftMagnitude));
				//log_d("%d", _TaskParams.fftMag[i]);
				if(pFreqPower[ind] > maxMag) {
					maxMag = pFreqPower[ind];
					maxBin=i;
				}
			}
		}
		else { //binRes==BinResolution::AUTO32
			uint16_t ind=1;
			int32_t mag=0;
			const uint8_t* pBinIndexes = _Auto32Groups;
			uint8_t currInd=0, upToInd=0;

			if(binRes == BinResolution::AUTO34) {
				maxIndex = 34;
				pBinIndexes = _Auto33Groups_v4;
			}
			else {
				maxIndex = 32;
				pBinIndexes = _Auto32Groups;
			}
			currInd=pBinIndexes[0];
			do {
				pFreqPower[0] = 0;
				// log_d("Bucle %d", ind);
				// upToInd = pBinIndexes[currInd];
				// do{

				// }while(currInd<);

				for(uint16_t k = pBinIndexes[ind-1]; k < pBinIndexes[ind]; k++) {
					mag = (int32_t)sqrt(pow(_pRealFftPlan->output[k * 2], 2) + pow(_pRealFftPlan->output[(k * 2) + 1], 2));
					if(mag > pFreqPower[ind]) {
						pFreqPower[ind]=mag;
					}
					// log_d("       Bucle %d - %d", ind,k);
				}
				// log_d("ind=%d avg=%d", ind, avg);
				pFreqPower[ind] = (10.0 * log((float)pFreqPower[ind] / (float)maxFftMagnitude));
				// log_d("%d - %d", ind, pFreqPower[ind]);

				if(pFreqPower[ind] > maxMag) {
					maxMag = pFreqPower[ind];
					maxBin = _Auto32Groups[ind-1];
				}
				ind++;
			} while(ind < maxIndex);
		}
	}
	// void NewAudio(uint16_t *pAudioIn, uint16_t samplesIn)
	// {

	// }

private:
	fft_config_t* _pRealFftPlan;
	uint16_t _numSamples;

	const uint8_t _Auto32Groups[32]={
		2 , 3, 4, 5, 6, 7, 9, 11,   //8
		13 ,15,17,19,21,23,26,29,   //12
		31,34,37,41,45,49,53,57,   //24
		61,65,73,81,89,97,105,128};//32

	const uint8_t _Auto33Groups[34] = {
		1,    2,  3,  4, 5, 6, 7, 8, 9,//8x1 ~379hz
		10,  12, 14, 16,18,20,22,      //7x2 ~924hz
		26,  30, 34, 38,42,46,         //6x4 ~1012Hz
		52,  58, 64, 70,76,            //5x6 ~3192hz
		83,  90, 97,104,               //4x7 ~4368Hz
		112,120,128                    //3x8 ~5376Hz
 };

	const uint8_t _Auto33Groups_v2[34] = {
		1,    2,  3,  4, 5, 6, 7, 8, 9,//8x1 ~379hz
		10,  12, 14, 16,18,20,22,24,   //8x2 ~1008hz
		27,  30, 33, 36,39,42,45,48,   //8x3 ~2016Hz
		53,  58, 63, 68,73,78,         //6x5 ~3276hz
		90,  107, 128                  //15-17-21 ~5376hz
	};

	const uint8_t _Auto33Groups_v3[34] = {
		    2,  3,  4, 5, 6, 7, 8, 9,//8x1 ~379hz
		10,  11, 12, 13,14,15,16,17,18,   //8x2 ~714hz
		19,  21, 23, 25,27,29,31,33,   //8x3 ~1386Hz
		36,  40, 45, 51,58,66,         //6x5 ~2772hz
		80,  100, 128                  //14-20-28 ~5376hz
	};

	const uint8_t _Auto33Groups_v4[34] = {
		 2,  3,  4, 5, 6, 7, 8, 9,10,//8x1 ~379hz
		11, 12, 13,14,15,16,17,18,   //8x2 ~714hz
		19, 20, 21, 22,23,24,26,28,   //8x3 ~1386Hz
		32, 34, 37, 40,45,55,         //6x5 ~2772hz
		80, 100, 128                  //14-20-28 ~5376hz
	};
};

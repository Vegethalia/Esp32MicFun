#include <Arduino.h>
#include <unity.h>
#include <stdlib.h>

#include "FftPower.h"
#include "../../src/FftPower.cpp"

#if !defined(ARDUINO)
fft_config_t* fft_init(int size, fft_type_t type, fft_direction_t direction, float* input, float* output) {
  fft_config_t* config = (fft_config_t*)malloc(sizeof(fft_config_t));
  if (!config || size <= 0) {
    free(config);
    return nullptr;
  }

  config->size = size;
  config->type = type;
  config->direction = direction;
  config->flags = 0;
  config->twiddle_factors = nullptr;

  if (input != nullptr) {
    config->input = input;
  } else {
    config->input = (float*)calloc(size, sizeof(float));
    config->flags |= FFT_OWN_INPUT_MEM;
  }

  if (output != nullptr) {
    config->output = output;
  } else {
    config->output = (float*)calloc(size, sizeof(float));
    config->flags |= FFT_OWN_OUTPUT_MEM;
  }

  if (!config->input || !config->output) {
    if (config->flags & FFT_OWN_INPUT_MEM) {
      free(config->input);
    }
    if (config->flags & FFT_OWN_OUTPUT_MEM) {
      free(config->output);
    }
    free(config);
    return nullptr;
  }

  return config;
}

void fft_destroy(fft_config_t* config) {
  if (!config) {
    return;
  }
  if (config->flags & FFT_OWN_INPUT_MEM) {
    free(config->input);
  }
  if (config->flags & FFT_OWN_OUTPUT_MEM) {
    free(config->output);
  }
  free(config);
}

void fft_execute(fft_config_t* config) {
  (void)config;
}
#endif

void setUp(void) {}
void tearDown(void) {}

void test_matrix_uses_interleaved_complex_indexes(void) {
  FftPower fft(64, 1);
  fft.ClearOutputForTest();

  // Bins sampled by MATRIX with numFreqsOut=4 and fftSize=64: 1, 8, 15, 22.
  fft.SetOutputBinForTest(1, 30.0f, 0.0f);
  fft.SetOutputBinForTest(8, 10.0f, 0.0f);
  fft.SetOutputBinForTest(15, 5.0f, 0.0f);
  fft.SetOutputBinForTest(22, 1.0f, 0.0f);

  // Sentinel: old buggy code read wrong indexes and could hit this value.
  fft.SetOutputBinForTest(4, 200.0f, 0.0f);

  int8_t power[4] = {-100, -100, -100, -100};
  uint16_t maxBin = 999;
  int32_t maxMag = -1;

  fft.GetFreqPower(power, 4, 1000, FftPower::MATRIX, maxBin, maxMag);

  TEST_ASSERT_EQUAL_UINT16(0, maxBin);
  TEST_ASSERT_TRUE(power[0] > power[1]);
}

void test_auto34_uses_band_maximum_and_db_scaling(void) {
  FftPower fft(256, 1);
  fft.ClearOutputForTest();

  // First AUTO34 band takes bin 2. Second band takes bin 3.
  fft.SetOutputBinForTest(2, 100.0f, 0.0f);
  fft.SetOutputBinForTest(3, 50.0f, 0.0f);

  int8_t power[34];
  for (int i = 0; i < 34; i++) {
    power[i] = -100;
  }
  uint16_t maxBin = 0;
  int32_t maxMag = -1;

  fft.GetFreqPower(power, 34, 1000, FftPower::AUTO34, maxBin, maxMag);

  TEST_ASSERT_EQUAL_UINT16(2, maxBin);
  TEST_ASSERT_TRUE(power[1] > power[2]);
  TEST_ASSERT_TRUE(power[1] <= -10 && power[1] >= -30);  // near -20dB
}

void test_auto34_db_is_clamped_to_zero_over_reference(void) {
  FftPower fft(256, 1);
  fft.ClearOutputForTest();
  fft.SetOutputBinForTest(2, 5000.0f, 0.0f);

  int8_t power[34];
  for (int i = 0; i < 34; i++) {
    power[i] = -100;
  }
  uint16_t maxBin = 0;
  int32_t maxMag = -1;

  fft.GetFreqPower(power, 34, 1000, FftPower::AUTO34, maxBin, maxMag);
  TEST_ASSERT_EQUAL_INT8(0, power[1]);
}

#if defined(ARDUINO)
void setup(void) {
  delay(1000);
  UNITY_BEGIN();
  RUN_TEST(test_matrix_uses_interleaved_complex_indexes);
  RUN_TEST(test_auto34_uses_band_maximum_and_db_scaling);
  RUN_TEST(test_auto34_db_is_clamped_to_zero_over_reference);
  UNITY_END();
}

void loop(void) {}
#else
int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_matrix_uses_interleaved_complex_indexes);
  RUN_TEST(test_auto34_uses_band_maximum_and_db_scaling);
  RUN_TEST(test_auto34_db_is_clamped_to_zero_over_reference);
  return UNITY_END();
}
#endif

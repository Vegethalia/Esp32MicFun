#ifndef __FFT_H__
#define __FFT_H__

typedef enum {
  FFT_REAL,
  FFT_COMPLEX
} fft_type_t;

typedef enum {
  FFT_FORWARD,
  FFT_BACKWARD
} fft_direction_t;

#define FFT_OWN_INPUT_MEM 1
#define FFT_OWN_OUTPUT_MEM 2

typedef struct {
  int size;
  float* input;
  float* output;
  float* twiddle_factors;
  fft_type_t type;
  fft_direction_t direction;
  unsigned int flags;
} fft_config_t;

fft_config_t* fft_init(int size, fft_type_t type, fft_direction_t direction, float* input, float* output);
void fft_destroy(fft_config_t* config);
void fft_execute(fft_config_t* config);

#endif

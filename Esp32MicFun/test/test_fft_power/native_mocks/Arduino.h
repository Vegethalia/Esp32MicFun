#pragma once

#include <algorithm>
#include <math.h>
#include <stdint.h>
#include <string.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef log_d
#define log_d(...) ((void)0)
#endif

inline void delay(unsigned long) {}

using std::max;
using std::min;

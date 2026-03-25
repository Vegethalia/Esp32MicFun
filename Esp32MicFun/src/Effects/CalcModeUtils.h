#pragma once

#include <stdint.h>

#include <string>

inline std::string GetVisibleRightDigits(const std::string& value, uint16_t consumedDigits, uint16_t visibleDigits) {
  if (visibleDigits <= consumedDigits) {
    return "";
  }

  uint16_t availableDigits = visibleDigits - consumedDigits;
  if (value.length() > availableDigits) {
    return value.substr(value.length() - availableDigits);
  }

  return value;
}

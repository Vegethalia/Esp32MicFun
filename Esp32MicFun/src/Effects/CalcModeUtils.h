#pragma once

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <vector>

struct CalcLongDivisionStep {
  uint32_t currentValue = 0;
  uint32_t subtractionValue = 0;
  uint32_t remainderValue = 0;
  uint8_t dividendIndex = 0;
};

struct CalcLongDivisionData {
  bool valid = false;
  std::string quotient;
  uint32_t remainder = 0;
  uint8_t quotientStartIndex = 0;
  std::vector<CalcLongDivisionStep> steps;
};

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

inline CalcLongDivisionData BuildLongDivision(const std::string& dividendStr, const std::string& divisorStr) {
  CalcLongDivisionData data;
  if (dividendStr.empty() || divisorStr.empty()) {
    return data;
  }

  uint32_t divisor = (uint32_t)strtoul(divisorStr.c_str(), nullptr, 10);
  if (divisor == 0) {
    return data;
  }

  uint32_t currentValue = 0;
  bool quotientStarted = false;
  for (uint8_t idx = 0; idx < dividendStr.length(); idx++) {
    currentValue = (currentValue * 10) + (uint32_t)(dividendStr[idx] - '0');
    if (!quotientStarted && currentValue < divisor) {
      continue;
    }

    quotientStarted = true;
    uint32_t quotientDigit = currentValue / divisor;
    CalcLongDivisionStep step;
    step.currentValue = currentValue;
    step.subtractionValue = quotientDigit * divisor;
    step.remainderValue = currentValue - step.subtractionValue;
    step.dividendIndex = idx;

    data.quotient.push_back((char)('0' + quotientDigit));
    data.steps.push_back(step);
    currentValue = step.remainderValue;
  }

  data.valid = true;
  data.remainder = currentValue;
  if (data.quotient.empty()) {
    data.quotient = "0";
    data.quotientStartIndex = dividendStr.length() > 0 ? (uint8_t)(dividendStr.length() - 1) : 0;
  } else {
    data.quotientStartIndex = (uint8_t)(dividendStr.length() - data.quotient.length());
  }

  return data;
}

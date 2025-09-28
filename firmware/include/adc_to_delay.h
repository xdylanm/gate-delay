#pragma once

#ifdef TEST_SUITE
#include <cstdint>
#else
#include <Arduino.h>
#endif

uint16_t adc_to_delay(uint16_t adc_val);
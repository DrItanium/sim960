#ifndef I960_TARGET_PLATFORM_H__
#define I960_TARGET_PLATFORM_H__
#if defined(ARDUINO_GRAND_CENTRAL_M4) || defined(ADAFRUIT_GRAND_CENTRAL_M4)
#include "GrandCentralM4.h"
#elif defined(ADAFRUIT_METRO_M4_EXPRESS) || defined(ARDUINO_METRO_M4)
#include "MetroM4Express.h"
#elif defined(ADAFRUIT_METRO_M0_EXPRESS) || defined(ARDUINO_METRO_M0)
#include "MetroM0Express.h"
#else
#error "Unknown Board"
#endif
#endif // end I960_TARGET_PLATFORM_H__

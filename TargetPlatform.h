#ifndef I960_TARGET_PLATFORM_H__
#define I960_TARGET_PLATFORM_H__
#include <type_traits>
#if defined(ARDUINO_GRAND_CENTRAL_M4) || defined(ADAFRUIT_GRAND_CENTRAL_M4)
#include "GrandCentralM4.h"
#elif defined(ADAFRUIT_METRO_M4_EXPRESS) || defined(ARDUINO_METRO_M4)
#include "MetroM4Express.h"
#elif defined(ADAFRUIT_METRO_M0_EXPRESS) || defined(ARDUINO_METRO_M0)
#include "MetroM0Express.h"
#elif defined(ADAFRUIT_TRINKET_M0) || defined(ARDUINO_TRINKET_M0)
#include "TrinketM0.h"
#else
#error "Unknown Board"
#endif
static_assert(std::is_default_constructible_v<TargetBoard>, "Board object must be default constructible");
#endif // end I960_TARGET_PLATFORM_H__

#ifndef I960_TARGET_PLATFORM_H__
#define I960_TARGET_PLATFORM_H__
#ifdef ARDUINO_GRAND_CENTRAL_M4
#include "GrandCentralM4.h"
#else
#error "Unknown Board"
#endif
#endif // end I960_TARGET_PLATFORM_H__

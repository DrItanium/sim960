#ifndef SETUP_SERIAL_H__
#define SETUP_SERIAL_H__
#include "SerialKind.h"

void setupSerial(int rate, IsHardwareBasedSerial);
void setupSerial(int rate, IsSoftwareBasedSerial);

template<typename SK>
void 
setupSerial(int rate) {
    setupSerial(rate, SK{ });
}
#endif // end SETUP_SERIAL_H__

#include "ControlSignals.h"

namespace i960 
{
    void setupSerial(int rate, IsHardwareBasedSerial) {
        Serial.begin(rate);
    }
    void setupSerial(int rate, IsSoftwareBasedSerial) {
        Serial.begin(rate);
        while(!Serial);
    }
}

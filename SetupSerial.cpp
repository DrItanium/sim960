#include <Arduino.h>
#include "SetupSerial.h"

void setupSerial(int rate, IsHardwareBasedSerial) {
    Serial.begin(rate);
}
void setupSerial(int rate, IsSoftwareBasedSerial) {
    Serial.begin(rate);
    while(!Serial);
}

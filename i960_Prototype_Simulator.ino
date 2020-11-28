#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <SD.h>
#include <SPI.h>
#include "TargetPlatform.h"
#include "PinSetup.h"
#include "Core.h"
i960::Core cpuCore;
/// @todo implement the register frames "in hardware"
void setup() {
    cpuCore.begin();
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    cpuCore.cycle(); // put a single cycle through
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

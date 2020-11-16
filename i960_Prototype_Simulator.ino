#include <SPI.h>
#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <SD.h>
#include "TargetPlatform.h"
#include "ControlSignals.h"
#include "PinSetup.h"


// targeting grand central m4
using Integer = int32_t;

using Ordinal = uint32_t;

using Register = Ordinal;

using RegisterSet = std::array<Register,16>;
volatile RegisterSet globalRegisters = { 0 };
volatile RegisterSet localRegisters = { 0 };
/// @todo implement the register frames "in hardware"
void setup() {
    i960::setupSerial(9600);
    Serial.println("i960 Simulator Starting up");
    i960::displayPinoutToConsole();
    i960::setupPins();
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
}

void loop() {
  // put your main code here, to run repeatedly:
    //Serial.println("Donuts");
    delay(1000);
}

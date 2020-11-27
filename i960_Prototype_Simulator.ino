#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <memory>
#include <SD.h>
#include <SPI.h>
#include "TargetPlatform.h"
#include "PinSetup.h"
#include "SetupSerial.h"
#include "Core.h"
#include "MemoryInterface.h"
std::unique_ptr<i960::Core> theCore = nullptr;
/// @todo implement the register frames "in hardware"
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    setupSerial(9600);
    Serial.println("i960 Simulator Starting up");
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
    theCore = std::make_unique<i960::Core>(i960::getStubMemoryController());
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    theCore->cycle();
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

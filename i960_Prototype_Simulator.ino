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
    setupSerial(9600);
    Serial.println("i960 Simulator Starting up");
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
    theCore = std::make_unique<i960::Core>(i960::getStubMemoryController());
}

void loop() {
    theCore->cycle();
  // put your main code here, to run repeatedly:
    //Serial.println("Donuts");
    delay(1);
}

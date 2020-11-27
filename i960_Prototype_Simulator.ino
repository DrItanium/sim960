#include <SPI.h>
#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <SD.h>
#include "TargetPlatform.h"
#include "PinSetup.h"
#include "SetupSerial.h"
#include "Core.h"

/// @todo implement the register frames "in hardware"
void setup() {
    setupSerial(9600);
    Serial.println("i960 Simulator Starting up");
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
}

void loop() {
  // put your main code here, to run repeatedly:
    //Serial.println("Donuts");
    delay(1000);
}

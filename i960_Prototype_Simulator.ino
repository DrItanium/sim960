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
  Serial.begin(9600);
  delay(1000);
  Serial.println("i960 Simulator Starting up");
  displayPinoutToConsole();
  setupPins();
  Serial.println("SPI Started");
  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}

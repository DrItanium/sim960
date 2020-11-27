#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <memory>
#include <SD.h>
#include <SPI.h>
#include "TargetPlatform.h"
#include "PinSetup.h"
#include "Core.h"
TargetBoard theBoard; // must be default constructible
std::unique_ptr<i960::Core> theCore = nullptr;
/// @todo implement the register frames "in hardware"
void setup() {
    theBoard.begin();
    theCore = std::make_unique<i960::Core>(theBoard);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    theCore->cycle();
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

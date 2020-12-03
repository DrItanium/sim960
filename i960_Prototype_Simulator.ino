#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <SD.h>
#include <SPI.h>
#include "PinSetup.h"
#include "Core.h"

// Adapt this class to the target microcontroller board
// Right now I am targeting a grand central m4

Adafruit_NeoPixel onboardNeoPixel(1, 88, NEO_GRB + NEO_KHZ800);

namespace i960 {
    /*
     * For now, the Grand Central M4 uses an SD Card for its memory with a small portion of the on board sram used for scratchpad / always
     * available memory. On board devices are mapped into the implicit onboard device area of [0xFF00'0000,0xFFFF'FFFF]. The IO device bus
     * exists here as extra "modules" such as:
     * - Clock Generator
     * - RTC
     * - ESP32 Wifi Coprocessor
     * - Display
     * - Sharp Memory Display
     * - GPIOs from the Grand Central
     * - SPI IO Expanders
     * - Speaker
     * - Control Registers
     * - 2 MB Onboard SPI Flash directly mapped into IO Device Space
     * - OPL3 Device
     * - Neopixels
     * - etc
     */
    Ordinal
    Core::loadOrdinal(Address address) noexcept {
    }

    Integer
    Core::loadInteger(Address address) noexcept {
    }

    ByteOrdinal
    Core::loadByteOrdinal(Address address) noexcept {
    }

    ByteInteger
    Core::loadByteInteger(Address address) noexcept {
    }

    ShortOrdinal
    Core::loadShortOrdinal(Address address) noexcept {
    }

    ShortInteger
    Core::loadShortInteger(Address address) noexcept {
    }

    void
    Core::storeOrdinal(Address address, Ordinal value) noexcept {
    }

    void
    Core::storeByteInteger(Address address, ByteInteger value) {
    }

    void
    Core::storeByteOrdinal(Address address, ByteOrdinal value) noexcept {
    }

    void
    Core::storeShortOrdinal(Address address, ShortOrdinal value) noexcept {
    }

    void
    Core::storeShortInteger(Address address, ShortInteger value) noexcept {
    }

    void
    Core::storeInteger(Address address, Integer value) noexcept {
    }
}
i960::Core cpuCore;
/// @todo implement the register frames "in hardware"
void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println("i960 Simulator Starting up");
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
    if (!SD.begin(SDCARD_SS_PIN)) {
        Serial.println("no sd card installed");
    } else {
        Serial.println("SD Card found");
    }
    onboardNeoPixel.begin();
    onboardNeoPixel.show();
    cpuCore.begin();
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    cpuCore.cycle(); // put a single cycle through
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

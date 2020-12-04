// we are defining a fake i960Zx series processor, this processor does not exist but will be what I'm calling my simulated design
// It has an SALIGN value of 4 like the i960Kx,i960Sx, and i960MC processors

/// @todo figure out the arcada library to allow for QSPI and SDCard simultaneous interaction and write an example for the Adafruit_SPIFlash library when done

// Adapt this class to the target microcontroller board
// Right now I am targeting a grand central m4
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include "PinSetup.h"
#include "Core.h"

constexpr auto i960Zx_SALIGN = 4;
Adafruit_NeoPixel onboardNeoPixel(1, 88, NEO_GRB + NEO_KHZ800);
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
SdFat sdCard; // use SDCARD_SS_PIN for this one
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
        return 0;
    }

    Integer
    Core::loadInteger(Address address) noexcept {
        return 0;
    }

    ByteOrdinal
    Core::loadByteOrdinal(Address address) noexcept {
        return 0;
    }

    ByteInteger
    Core::loadByteInteger(Address address) noexcept {
        return 0;
    }

    ShortOrdinal
    Core::loadShortOrdinal(Address address) noexcept {
        return 0;
    }

    ShortInteger
    Core::loadShortInteger(Address address) noexcept {
        return 0;
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
i960::Core cpuCore(i960Zx_SALIGN);
/// @todo implement the register frames "in hardware"
void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }
    Serial.println("i960 Simulator Starting up");
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
    Serial.print("Starting up onboard QSPI Flash...");
    flash.begin();
    Serial.println("Done");
    Serial.println("Onboard Flash information");
    Serial.print("JEDEC ID: 0x");
    Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.print(flash.size() / 1024);
    Serial.println(" KB");
    Serial.print("Initializing fileysstem on external flash...");
    if (!fatfs.begin(&flash)) {
        Serial.println("Error: filesystem does not exist! Please try SdFat_format example to make one!");
        while (1) {
            yield();
        }
    }
    Serial.println("Done");
    Serial.print("Starting up SD Card...");
    if (!sdCard.begin(SDCARD_SS_PIN)) {
        Serial.println("No card found");
    } else {
        Serial.println("Card found!");
    }
    onboardNeoPixel.begin();
    onboardNeoPixel.show();
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    cpuCore.cycle(); // put a single cycle through
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

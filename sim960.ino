// we are defining a fake i960Zx series processor, this processor does not exist but will be what I'm calling my simulated design
// It has an SALIGN value of 4 like the i960Kx,i960Sx, and i960MC processors

/// @todo figure out the arcada library to allow for QSPI and SDCard simultaneous interaction and write an example for the Adafruit_SPIFlash library when done

// Adapt this class to the target microcontroller board
// Right now I am targeting a grand central m4
#include <SPI.h>
#include <Wire.h>
#include <SdFat.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SPIFlash.h>
#include <ArduinoJson.h>
#include <OPL3Duo.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_SI5351.h>
#include <array>
#include "CoreTypes.h"
#include "Core.h"

constexpr auto OPL3Duo_A2 = 22;
constexpr auto OPL3Duo_A1 = 23;
constexpr auto OPL3Duo_A0 = 24;
constexpr auto OPL3Duo_Latch = 25;
constexpr auto OPL3Duo_Reset = 26;
constexpr auto i960Zx_SALIGN = 4;
Adafruit_NeoPixel onboardNeoPixel(1, 88, NEO_GRB + NEO_KHZ800);
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
SdFat sdCard; // use SDCARD_SS_PIN for this one
OPL3Duo theOPL3Duo(OPL3Duo_A2,
                   OPL3Duo_A1,
                   OPL3Duo_A0,
                   OPL3Duo_Latch,
                   OPL3Duo_Reset);
Adafruit_seesaw theSoilSensor;
Adafruit_SI5351 clockgen;
union MemoryCell {
    constexpr explicit MemoryCell(Ordinal value = 0) : oval(value) { }
    Ordinal oval = 0;
    Integer ival;
    ByteOrdinal bo[4];
    ByteInteger bi[4];
    ShortOrdinal so[2];
    ShortInteger si[2];

};
static_assert(sizeof(Ordinal) == sizeof(MemoryCell));
std::array<MemoryCell, 2048 / sizeof(MemoryCell)> dataRam;
// the i960 has other registers and tables we need to be aware of so onboard sram will most likely _not_ be exposed to the i960 processor
// directly
namespace i960 {
    /*
     * Memory Map So far (broken up into 16 megabyte blocks)
     * 0x0000'0000 - 0xFEFF'FFFF: FRAM / SD Card memory / memory buffer /etc
     * 0xFF00'0000 - 0xFFFF'FFFF: Internal IO Space (Where I expose the SAMD51 peripherals)
     */
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
     * - EEPROM
     * - FLASH memory
     * - etc
     */
    MemoryCell&
    loadCell(Address address) noexcept {

    }
    Ordinal
    Core::loadOrdinal(Address address) noexcept {
        auto cell = loadCell(address);
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
    void
    Core::badInstruction(DecodedInstruction inst) {
        Serial.println("BAD INSTRUCTION!");
        std::visit([](auto&& value) {
            using K = std::decay_t<decltype(value)>;
            Serial.print("Instruction opcode: 0x");
            if constexpr (std::is_same_v<K, i960::MEMFormatInstruction>) {
                Serial.print(value.upperHalf(), HEX);
            }
            Serial.println(value.lowerHalf(), HEX);
            auto name = value.decodeName();
            if (!name.empty()) {
                Serial.print("Name: ");
                Serial.println(name.c_str());
            }
        }, inst);
        raiseFault();
    }
}
i960::Core cpuCore(0, i960Zx_SALIGN);
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
    Serial.print("Initialzing onboard NeoPixel...");
    onboardNeoPixel.begin();
    onboardNeoPixel.show();
    Serial.println("Done");
    Serial.print("Starting up OPL3Duo...");
    theOPL3Duo.begin();
    Serial.println("Done");
    Serial.print("Bringing up the soil sensor...");
    if (!theSoilSensor.begin(0x36)) {
        Serial.println("Soil Sensor Not Found!");
    } else {
        Serial.println("Soil Sensor Found!");
        Serial.println(theSoilSensor.getVersion(), HEX);
    }

    Serial.print("Bringing up Si5351 Clock Chip...");
    if (clockgen.begin() != ERROR_NONE) {
        Serial.println("No Clock Chip Detected ... forget to hook it up?");
    } else {
        Serial.println("Done");
    }
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    cpuCore.cycle(); // put a single cycle through
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

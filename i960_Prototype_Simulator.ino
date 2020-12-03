#include <array>
#include <type_traits>
#include <string>
#include <map>
#include <SD.h>
#include <SPI.h>
#include "PinSetup.h"
#include "MemoryInterface.h"
#include "Core.h"
#include "HasOnboardNeoPixel.h"

// Adapt this class to the target microcontroller board
// Right now I am targeting a grand central m4
class TargetBoard : public MemoryInterface, public HasOnboardNeoPixel<88> {
public:
    TargetBoard() : MemoryInterface(), HasOnboardNeoPixel<88>() { }
    ~TargetBoard() override = default;
    Integer loadValue(Address address, TreatAsInteger) override;
    Ordinal loadValue(Address address, TreatAsOrdinal) override;
    void storeValue(Address address, Ordinal value, TreatAsOrdinal) override;
    void storeValue(Address address, Integer value, TreatAsInteger) override;
    ByteInteger loadValue(Address address, TreatAsByteInteger) override;
    ByteOrdinal loadValue(Address address, TreatAsByteOrdinal) override;
    void storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal) override;
    void storeValue(Address address, ByteInteger value, TreatAsByteInteger) override;
    ShortInteger loadValue(Address address, TreatAsShortInteger) override;
    ShortOrdinal loadValue(Address address, TreatAsShortOrdinal) override;
    void storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal) override;
    void storeValue(Address address, ShortInteger value, TreatAsShortInteger) override;
    void begin() override;
private:
    bool _initialized = false;
};
Integer
TargetBoard::loadValue(Address address, TreatAsInteger)  {
    return 0;
}
Ordinal
TargetBoard::loadValue(Address address, TreatAsOrdinal)  {
    return 0;
}
void
TargetBoard::storeValue(Address address, Ordinal value, TreatAsOrdinal) {

}
void
TargetBoard::storeValue(Address address, Integer value, TreatAsInteger)  {

}
ByteInteger
TargetBoard::loadValue(Address address, TreatAsByteInteger)  {
    return 0;
}
ByteOrdinal
TargetBoard::loadValue(Address address, TreatAsByteOrdinal)  {
    return 0;
}
void
TargetBoard::storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal)  {

}
void
TargetBoard::storeValue(Address address, ByteInteger value, TreatAsByteInteger)  {

}
ShortInteger
TargetBoard::loadValue(Address address, TreatAsShortInteger)  {
    return 0;
}
ShortOrdinal
TargetBoard::loadValue(Address address, TreatAsShortOrdinal)  {
    return 0;
}
void
TargetBoard::storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal)  {

}
void
TargetBoard::storeValue(Address address, ShortInteger value, TreatAsShortInteger)  {

}

void
TargetBoard::begin() {
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
    if (!_initialized) {
        _initialized = true;
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
        HasOnboardNeoPixel<88>::begin();
    }
}
i960::Core<TargetBoard> cpuCore;
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

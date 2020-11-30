//
// Created by jwscoggins on 11/27/20.
//
#include "GrandCentralM4.h"
#include <SPI.h>
#include <Arduino.h>
#include <SD.h>

Integer
GrandCentralM4Board::loadValue(Address address, TreatAsInteger)  {
    return 0;
}
Ordinal
GrandCentralM4Board::loadValue(Address address, TreatAsOrdinal)  {
    return 0;
}
void
GrandCentralM4Board::storeValue(Address address, Ordinal value, TreatAsOrdinal) {

}
void
GrandCentralM4Board::storeValue(Address address, Integer value, TreatAsInteger)  {

}
ByteInteger
GrandCentralM4Board::loadValue(Address address, TreatAsByteInteger)  {
    return 0;
}
ByteOrdinal
GrandCentralM4Board::loadValue(Address address, TreatAsByteOrdinal)  {
    return 0;
}
void
GrandCentralM4Board::storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal)  {

}
void
GrandCentralM4Board::storeValue(Address address, ByteInteger value, TreatAsByteInteger)  {

}
ShortInteger
GrandCentralM4Board::loadValue(Address address, TreatAsShortInteger)  {
    return 0;
}
ShortOrdinal
GrandCentralM4Board::loadValue(Address address, TreatAsShortOrdinal)  {
    return 0;
}
void
GrandCentralM4Board::storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal)  {

}
void
GrandCentralM4Board::storeValue(Address address, ShortInteger value, TreatAsShortInteger)  {

}

void
GrandCentralM4Board::begin() {
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
        if constexpr (hasBuiltinSDCard()) {
            if (!SD.begin(SDCARD_SS_PIN)) {
                Serial.println("no sd card installed");
            } else {
                Serial.println("SD Card found");
            }
        }
    }
}


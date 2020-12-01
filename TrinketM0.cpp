//
// Created by jwscoggins on 11/27/20.
//
#if defined(ARDUINO_TRINKET_M0) || defined(ADAFRUIT_TRINKET_M0)
#include "TrinketM0.h"
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_DotStar.h>

constexpr auto OnBoardDotstarNumPixels = 1;
constexpr auto OnBoardDotstarDataPin = 7;
constexpr auto OnBoardDotstarClockPin = 7;

Adafruit_DotStar onboardStrip(OnBoardDotstarNumPixels, OnBoardDotstarDataPin, OnBoardDotstarClockPin, DOTSTAR_BGR);

Integer
TrinketM0Board::loadValue(Address address, TreatAsInteger)  {
    return 0;
}
Ordinal
TrinketM0Board::loadValue(Address address, TreatAsOrdinal)  {
    return 0;
}
void
TrinketM0Board::storeValue(Address address, Ordinal value, TreatAsOrdinal) {

}
void
TrinketM0Board::storeValue(Address address, Integer value, TreatAsInteger)  {

}
ByteInteger
TrinketM0Board::loadValue(Address address, TreatAsByteInteger)  {
    return 0;
}
ByteOrdinal
TrinketM0Board::loadValue(Address address, TreatAsByteOrdinal)  {
    return 0;
}
void
TrinketM0Board::storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal)  {

}
void
TrinketM0Board::storeValue(Address address, ByteInteger value, TreatAsByteInteger)  {

}
ShortInteger
TrinketM0Board::loadValue(Address address, TreatAsShortInteger)  {
    return 0;
}
ShortOrdinal
TrinketM0Board::loadValue(Address address, TreatAsShortOrdinal)  {
    return 0;
}
void
TrinketM0Board::storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal)  {

}
void
TrinketM0Board::storeValue(Address address, ShortInteger value, TreatAsShortInteger)  {

}

void
TrinketM0Board::begin() {
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
        Serial.print("Starting up onboard DotStar LED...");
        onboardStrip.begin();
        onboardStrip.show();
        Serial.println("Done");
        onboardStrip.setPixelColor(0, 0x7F007F); // purple :)
    }
}
#endif // end defined(ARDUINO_GRAND_CENTRAL_M4) || defined(ADAFRUIT_GRAND_CENTRAL_M4)


//
// Created by jwscoggins on 11/27/20.
//
#if defined(ARDUINO_TRINKET_M0) || defined(ADAFRUIT_TRINKET_M0)
#include "CoreTypes.h"
#include "TrinketM0.h"
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_DotStar.h>
constexpr auto FRAM_ENABLE = 0;
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
    if (!_initialized) {
        _initialized = true;
        Serial.begin(9600);
        while (!Serial);
        Serial.println("i960 Simulator Starting up");
        pinMode(LED_BUILTIN, OUTPUT);
        pinMode(FRAM_ENABLE, OUTPUT);
        digitalWrite(FRAM_ENABLE, HIGH);
        Serial.print("Starting up SPI...");
        SPI.begin();
        Serial.println("Done");
        Serial.print("Starting up onboard DotStar LED...");
        HasOnboardDotStar::begin();
        setDotstarPixelValue(0,0,0);
        delay(1000);
        setDotstarPixelValue(0x7F,0, 0x7F);
        delay(1000);
        setDotstarPixelValue(0,0,0);
        Serial.println("Done");
    }
}
#endif // end defined(ARDUINO_GRAND_CENTRAL_M4) || defined(ADAFRUIT_GRAND_CENTRAL_M4)


//
// Created by jwscoggins on 11/27/20.
//
#if defined(ADAFRUIT_METRO_M4_EXPRESS) || defined(ARDUINO_METRO_M4)
#include "MetroM4Express.h"
#include <SPI.h>
#include <Arduino.h>
#include <SD.h>

Integer
MetroM4ExpressBoard::loadValue(Address address, TreatAsInteger)  {
    return 0;
}
Ordinal
MetroM4ExpressBoard::loadValue(Address address, TreatAsOrdinal)  {
    return 0;
}
void
MetroM4ExpressBoard::storeValue(Address address, Ordinal value, TreatAsOrdinal) {

}
void
MetroM4ExpressBoard::storeValue(Address address, Integer value, TreatAsInteger)  {

}
ByteInteger
MetroM4ExpressBoard::loadValue(Address address, TreatAsByteInteger)  {
    return 0;
}
ByteOrdinal
MetroM4ExpressBoard::loadValue(Address address, TreatAsByteOrdinal)  {
    return 0;
}
void
MetroM4ExpressBoard::storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal)  {

}
void
MetroM4ExpressBoard::storeValue(Address address, ByteInteger value, TreatAsByteInteger)  {

}
ShortInteger
MetroM4ExpressBoard::loadValue(Address address, TreatAsShortInteger)  {
    return 0;
}
ShortOrdinal
MetroM4ExpressBoard::loadValue(Address address, TreatAsShortOrdinal)  {
    return 0;
}
void
MetroM4ExpressBoard::storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal)  {

}
void
MetroM4ExpressBoard::storeValue(Address address, ShortInteger value, TreatAsShortInteger)  {

}

void
MetroM4ExpressBoard::begin() {
    // Unlike the grand central m4, the metro m4 does not have a built in sd card slot, I will need to use a shield for this purpose instead.
    // Adafruit has a wonderful shield for just this purpose and I do have one :). It comes with a builtin screen as well!
    if (!_initialized) {
        _initialized = true;
        Serial.begin(9600);
        while (!Serial);
        Serial.println("i960 Simulator Starting up");
        pinMode(LED_BUILTIN, OUTPUT);
        Serial.print("Starting up SPI...");
        SPI.begin();
        Serial.println("Done");
    }
}
#endif // defined ADAFRUIT_METRO_M4_EXPRESS || defined ARDUINO_METRO_M4


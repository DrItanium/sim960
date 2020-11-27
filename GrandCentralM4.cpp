//
// Created by jwscoggins on 11/27/20.
//
#include "GrandCentralM4.h"
#include <SPI.h>
#include <Arduino.h>
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
    Serial.begin(9600);
    while(!Serial);
    Serial.println("i960 Simulator Starting up");
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
}


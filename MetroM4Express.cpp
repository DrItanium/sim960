//
// Created by jwscoggins on 11/27/20.
//
#if defined(ADAFRUIT_METRO_M4_EXPRESS) || defined(ARDUINO_METRO_M4)
#include "MetroM4Express.h"
#include <SPI.h>
#include <Arduino.h>
#include <SD.h>

// this design has the adafruit 1.8" TFT shield with microsd card 5-way nav + 3 buttons shield attached to the top

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
MetroM4ExpressBoard::startupTFTShield() {
   pinMode(TFT_CS, OUTPUT);
   digitalWrite(TFT_CS, HIGH);
   pinMode(SD_CS, OUTPUT);
   digitalWrite(SD_CS, HIGH);

   if (!ss.begin()) {
       Serial.println("seesaw could not be initialized!");
       while(true);
   }
   Serial.println("seesaw started");
   Serial.print("Version: ");
   Serial.println(ss.getVersion(), HEX);

   // disable the backlight to start
   ss.setBacklight(TFTSHIELD_BACKLIGHT_OFF);
   ss.tftReset();

   tft.initR(INITR_BLACKTAB);
   Serial.println("TFT OK!");
   tft.fillScreen(ST77XX_GREEN);

   Serial.print("Initializing SD Card...");
   if (!SD.begin(SD_CS)) {
       Serial.println("failed!");
   } else {
       Serial.println("OK!");
   }

   for (int32_t i = TFTSHIELD_BACKLIGHT_OFF; i < TFTSHIELD_BACKLIGHT_ON; i+= 100) {
       ss.setBacklight(i);
       delay(1);
   }
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
        startupTFTShield();
        HasOnboardNeoPixel::begin();
        setNeoPixelColor(0x7F, 0, 0x7F);
        delay(1000);
        setNeoPixelColor(0, 0, 0);
    }
}

MetroM4ExpressBoard::MetroM4ExpressBoard() : MemoryInterface(), HasOnboardNeoPixel(40, NEO_GRB + NEO_KHZ800), tft(TFT_CS, TFT_DC, TFT_RESET) { }
#endif // defined ADAFRUIT_METRO_M4_EXPRESS || defined ARDUINO_METRO_M4


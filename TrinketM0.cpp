//
// Created by jwscoggins on 11/27/20.
//
#define ARDUINO_TRINKET_M0
#if defined(ARDUINO_TRINKET_M0) || defined(ADAFRUIT_TRINKET_M0)
#include "CoreTypes.h"
#include "TrinketM0.h"
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_DotStar.h>
constexpr auto FRAM_ENABLE = 0;
enum class FM25V20A_Opcodes : uint8_t {
  WREN = 0b00000110,
  WRDI = 0b00000100,
  RDSR = 0b00000101,
  WRSR = 0b00000001,
  READ = 0b00000011,
  FSTRD = 0b00001011,
  WRITE = 0b00000010,
  SLEEP = 0b10111001,
  RDID = 0b10011111,
};
template<typename T>
void sendOpcode(T opcode) noexcept {
    SPI.transfer(static_cast<uint8_t>(opcode));
}
void transferAddress(Ordinal address) {
    SPI.transfer(static_cast<uint8_t>(address >> 16));
    SPI.transfer(static_cast<uint8_t>(address >> 8));
    SPI.transfer(static_cast<uint8_t>(address));
}
template<int holdPin>
uint8_t
read8(uint32_t address) noexcept {
    digitalWrite(holdPin, LOW);
    sendOpcode(FM25V20A_Opcodes::READ);
    transferAddress(address);
    auto result = SPI.transfer(0);
    digitalWrite(holdPin, HIGH);
    return static_cast<uint8_t>(result);
}

template<int holdPin>
uint16_t
read16(uint32_t address) noexcept {
    digitalWrite(holdPin, LOW);
    sendOpcode(FM25V20A_Opcodes::READ);
    transferAddress(address & (~0b1));
    auto lower = SPI.transfer(0);
    auto upper = SPI.transfer(0);
    digitalWrite(holdPin, HIGH);
    return (static_cast<uint16_t>(upper) << 8) |
           (static_cast<uint16_t>(lower));
}
template<int holdPin>
uint32_t
read32(uint32_t address) noexcept {
    digitalWrite(holdPin, LOW);
    sendOpcode(FM25V20A_Opcodes::READ);
    transferAddress((address & (~0b11)));
    auto a = static_cast<uint32_t>(SPI.transfer(0));
    auto b = static_cast<uint32_t>(SPI.transfer(0)) << 8;
    auto c = static_cast<uint32_t>(SPI.transfer(0)) << 16;
    auto d = static_cast<uint32_t>(SPI.transfer(0)) << 24;
    digitalWrite(holdPin, HIGH);
    return a | b | c | d;
}
template<int holdPin>
uint64_t
read64(uint32_t address) noexcept {
    digitalWrite(holdPin, LOW);
    sendOpcode(FM25V20A_Opcodes::READ);
    transferAddress(address & (~0b111));
    auto a = static_cast<uint64_t>(SPI.transfer(0));
    auto b = static_cast<uint64_t>(SPI.transfer(0)) << 8;
    auto c = static_cast<uint64_t>(SPI.transfer(0)) << 16;
    auto d = static_cast<uint64_t>(SPI.transfer(0)) << 24;
    auto e = static_cast<uint64_t>(SPI.transfer(0)) << 32;
    auto f = static_cast<uint64_t>(SPI.transfer(0)) << 40;
    auto g = static_cast<uint64_t>(SPI.transfer(0)) << 48;
    auto h = static_cast<uint64_t>(SPI.transfer(0)) << 56;
    digitalWrite(holdPin, HIGH);
    return a | b | c | d | e | f | g | h;
}
template<int pin>
void enableWrites() {
    digitalWrite(pin, LOW);
    sendOpcode(FM25V20A_Opcodes::WREN);
    digitalWrite(pin, HIGH);
}
template<int pin>
void writeFram(uint32_t address, uint8_t value) {
    enableWrites<pin>();
    digitalWrite(pin, LOW);
    sendOpcode(FM25V20A_Opcodes::WRITE);
    transferAddress(address);
    SPI.transfer(value);
    digitalWrite(pin, HIGH);
}
template<int pin>
void writeFram(uint32_t address, uint16_t value) {
    enableWrites<pin>();
    digitalWrite(pin, LOW);
    sendOpcode(FM25V20A_Opcodes::WRITE);
    transferAddress(address);
    SPI.transfer(static_cast<uint8_t>(value));
    SPI.transfer(static_cast<uint8_t>(value >> 8));
    digitalWrite(pin, HIGH);
}
template<int pin>
void writeFram(uint32_t address, uint32_t value) {
    enableWrites<pin>();
    digitalWrite(pin, LOW);
    sendOpcode(FM25V20A_Opcodes::WRITE);
    transferAddress(address);
    SPI.transfer(static_cast<uint8_t>(value));
    SPI.transfer(static_cast<uint8_t>(value >> 8));
    SPI.transfer(static_cast<uint8_t>(value >> 16));
    SPI.transfer(static_cast<uint8_t>(value >> 24));
    digitalWrite(pin, HIGH);
}

template<int pin>
void writeFram(uint32_t address, uint64_t value) {
    enableWrites<pin>();
    digitalWrite(pin, LOW);
    sendOpcode(FM25V20A_Opcodes::WRITE);
    transferAddress(address);
    SPI.transfer(static_cast<uint8_t>(value));
    SPI.transfer(static_cast<uint8_t>(value >> 8));
    SPI.transfer(static_cast<uint8_t>(value >> 16));
    SPI.transfer(static_cast<uint8_t>(value >> 24));
    SPI.transfer(static_cast<uint8_t>(value >> 32));
    SPI.transfer(static_cast<uint8_t>(value >> 40));
    SPI.transfer(static_cast<uint8_t>(value >> 48));
    SPI.transfer(static_cast<uint8_t>(value >> 56));
    digitalWrite(pin, HIGH);
}
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
TrinketM0Board::testFRAMArray() {
    // do a simple round trip check
    Serial.print("Testing fram array...");
    for (Ordinal i = 0; i < 0x40000; ++i)  {
        auto check = static_cast<uint8_t>(i);
        writeFram<FRAM_ENABLE>(i, check);
        if (auto value = read8<FRAM_ENABLE>(i); value != check) {
            setDotstarPixelValue(0xFF, 0, 0);
            delay(1000);
            setDotstarPixelValue(0,0,0);
        }
    }
    Serial.println("Done");
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
        testFRAMArray();
    }
}
#endif // end defined(ARDUINO_GRAND_CENTRAL_M4) || defined(ADAFRUIT_GRAND_CENTRAL_M4)


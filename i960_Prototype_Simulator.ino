#include <SPI.h>
#include <array>
#include <type_traits>
#include <string>
#include <map>
#include "TargetPlatform.h"

template<typename ... Pins>
void setBlockOfPins(decltype(OUTPUT) mode, decltype(HIGH) value, Pins ... pins) noexcept {
    /// @todo Once we have std::move and std::forward, make pins &&
    auto fn = [mode, value](auto pin) noexcept {
        pinMode(static_cast<int>(pin), mode);
        if (mode == OUTPUT) {
            digitalWrite(static_cast<int>(pin), value);
        }
    };
    (fn(pins), ...);
}

template<typename ... Pins>
void setBlockOfOutputPins(Pins ... pins) noexcept {
    /// @todo Once we have std::move and std::forward, make pins &&
    setBlockOfPins(OUTPUT, HIGH, pins...);
}

template<typename ... Pins>
void setBlockOfInputPins(Pins ... pins) noexcept {
    setBlockOfPins(INPUT, LOW, pins...);
}

template<typename ... Pins>
void setBlockOfInputPullupPins(Pins ... pins) noexcept {
    setBlockOfPins(INPUT, HIGH, pins...);
}

// targeting grand central m4
using Integer = int32_t;

using Ordinal = uint32_t;

using Register = Ordinal;

using RegisterSet = std::array<Register,16>;
volatile RegisterSet globalRegisters = { 0 };
volatile RegisterSet localRegisters = { 0 };
/// @todo implement the register frames "in hardware"
template<typename T>
std::string 
getPinName(T value) noexcept {
    static std::map<T, std::string> lookupTable = {
        {T::Lock, "Lock_" },
        {T::INT0, "INT0_" },
        {T::INT1, "INT1" },
        {T::INT2, "INT2" },
        {T::INT3, "INT3_" },
        {T::Ready, "READY_" },
        {T::Hold, "HOLD" }, 
        {T::ALE, "ALE"},
        {T::AS, "AS_" },
        {T::BLAST, "BLAST_"},
        {T::DT_R, "DT/R_" },
        {T::DEN, "DEN_" },
        {T::W_R, "W/R_" },
        {T::HLDA, "HLDA" },
        {T::BE0, "BE0_"},
        {T::BE1, "BE1_"},
        {T::BA1, "BA1"},
        {T::BA2, "BA2"},
        {T::BA3, "BA3"},
        {T::Last, "Last" },
    };
    static_assert(std::is_enum_v<T>, "Pinout type must be an enum");
    if (auto result = lookupTable.find(value); result != lookupTable.end()) {
        return result->second;
    } else {
        return "UNKNOWN";
    }
}
template<typename T>
void displayPinoutToConsole() noexcept {
    static_assert(std::is_enum_v<T>, "Pinout type must be an enum");
    for (auto i = T::First; i != T::Last; i = static_cast<T>(static_cast<int>(i) + 1)) {
        auto name = getPinName(i);
        Serial.print(name.c_str());
        Serial.print(": ");
        Serial.println(static_cast<int>(i));
    }
}
template<typename T>
void setupPins() noexcept {
  // put your setup code here, to run once:
  setBlockOfInputPins(T::Lock, 
                      T::INT3); // open collector
  setBlockOfInputPins(
          T::INT0, 
          T::INT1, 
          T::INT2, 
          T::Ready, 
          T::Hold);
  setBlockOfOutputPins(
          T::ALE,
          T::AS,
          T::BLAST,
          T::DT_R,
          T::DEN,
          T::W_R,
          T::HLDA,
          T::BE0,
          T::BE1,
          T::BA1,
          T::BA2,
          T::BA3);
}
void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("i960 Simulator Starting up");
  displayPinoutToConsole<TargetBoardPinout>();
  setupPins<TargetBoardPinout>();
  Serial.println("SPI Started");
  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}

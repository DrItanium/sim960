#ifndef I960_CONTROL_SIGNALS_H__
#define I960_CONTROL_SIGNALS_H__
#include <string>
#include <map>
#include "TargetPlatform.h"
#include "PinSetup.h"

template<typename T>
std::string 
getPinName(T value) noexcept {
    static std::map<T, std::string> lookupTable {
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
template<typename T = TargetBoardPinout>
void displayPinoutToConsole() noexcept {
    static_assert(std::is_enum_v<T>, "Pinout type must be an enum");
    for (auto i = T::First; i != T::Last; i = static_cast<T>(static_cast<int>(i) + 1)) {
        auto name = getPinName(i);
        Serial.print(name.c_str());
        Serial.print(": ");
        Serial.println(static_cast<int>(i));
    }
}

template<typename T = TargetBoardPinout>
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
#endif // end I960_CONTROL_SIGNALS_H__

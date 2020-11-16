#include <array>
#include <type_traits>
#include <string>
#include <map>

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
enum class GrandCentralM4Pinout : uint8_t {
  Lock = 22, // active low, open collector, bi directional
  INT0, // active low input
  INT1, // active low 
  INT2,
  INT3,
  Ready,
  Hold,
  ALE,
  AS,
  BLAST,
  DT_R,
  DEN,
  W_R,
  HLDA,
  BE0,
  BE1,
  BA1,
  BA2,
  BA3,
  // continue here
  Last,
  First = Lock,
};
template<typename T>
std::string 
getPinName(T value) noexcept {
    static std::map<T, std::string> lookupTable = {
        {T::Lock, "Lock_" },
        {T::INT0, "INT0_" },
        {T::INT1, "INT1" },
        {T::INT2, "INT2" },
        {T::INT3_, "INT3_" },
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
        Serial.print(getPinName(i));
        Serial.print(": ");
        Serial.println(static_cast<int>(i));
    }
}
void setup() {
  Serial.begin(115200);
  Serial.println("i960 Simulator Starting up");
  // put your setup code here, to run once:
  setBlockOfInputPins(GrandCentralM4Pinout::Lock, 
                      GrandCentralM4Pinout::INT3); // open collector
  setBlockOfInputPins(
          GrandCentralM4Pinout::INT0, 
          GrandCentralM4Pinout::INT1, 
          GrandCentralM4Pinout::INT2, 
          GrandCentralM4Pinout::Ready, 
          GrandCentralM4Pinout::Hold);
  setBlockOfOutputPins(
          GrandCentralM4Pinout::ALE,
          GrandCentralM4Pinout::AS,
          GrandCentralM4Pinout::BLAST,
          GrandCentralM4Pinout::DT_R,
          GrandCentralM4Pinout::DEN,
          GrandCentralM4Pinout::W_R,
          GrandCentralM4Pinout::HLDA,
          GrandCentralM4Pinout::BE0,
          GrandCentralM4Pinout::BE1,
          GrandCentralM4Pinout::BA1,
          GrandCentralM4Pinout::BA2,
          GrandCentralM4Pinout::BA3);
}

void loop() {
  // put your main code here, to run repeatedly:

}

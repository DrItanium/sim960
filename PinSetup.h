#ifndef PIN_SETUP_H__
#define PIN_SETUP_H__
#include <Arduino.h>
#include <type_traits>

template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
void pinMode(T pin, decltype(INPUT) mode) noexcept {
    pinMode(static_cast<int>(pin), mode);
}
template<typename T, std::enable_if_t<std::is_enum_v<T>,int > = 0 >
void digitalWrite(T pin, decltype(HIGH) value) noexcept {
    digitalWrite(static_cast<int>(pin), value);
}
template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
decltype(HIGH) digitalRead(T pin) noexcept {
    return digitalRead(static_cast<int>(pin));
}
template<typename ... Pins>
void setBlockOfPins(decltype(OUTPUT) mode, decltype(HIGH) value, Pins ... pins) noexcept {
    /// @todo Once we have std::move and std::forward, make pins &&
    auto fn = [mode, value](auto pin) noexcept {
        pinMode(pin, mode);
        if (mode == OUTPUT) {
            digitalWrite(pin, value);
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
#endif // end PIN_SETUP_H__

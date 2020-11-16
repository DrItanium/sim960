#ifndef PIN_SETUP_H__
#define PIN_SETUP_H__
#include <Arduino.h>
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
#endif // end PIN_SETUP_H__

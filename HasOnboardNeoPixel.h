//
// Created by jwscoggins on 12/1/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H
#define I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
class HasOnboardNeoPixel {
public:
    HasOnboardNeoPixel(int pinIndex, int flags);
    void begin();
    auto& get() noexcept { return np; }
    void setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b, bool update = true) noexcept;
    void updateNeoPixel() noexcept;
private:
    Adafruit_NeoPixel np;
};

#endif //I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H

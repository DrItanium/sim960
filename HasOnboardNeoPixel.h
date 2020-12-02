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
    auto& get() noexcept { return np; }
private:
    Adafruit_NeoPixel np;
};

#endif //I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H

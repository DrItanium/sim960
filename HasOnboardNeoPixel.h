//
// Created by jwscoggins on 12/1/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H
#define I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
template<int pinIndex, int flags = NEO_GRB + NEO_KHZ800, int numOnboardPixels = 1>
class HasOnboardNeoPixel {
public:
    HasOnboardNeoPixel() : np(numOnboardPixels, pinIndex, flags) { };
    void begin() {
        np.begin();
        np.show();
    }
    auto& get() noexcept { return np; }
    void
    setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b, bool update = true) noexcept {
        np.setPixelColor(0, r, g, b);
        if (update) {
            np.show();
        }
    }
    void
    updateNeoPixel() noexcept {
        np.show();
    }
private:
    Adafruit_NeoPixel np;
};

#endif //I960_PROTOTYPE_SIMULATOR_HASONBOARDNEOPIXEL_H

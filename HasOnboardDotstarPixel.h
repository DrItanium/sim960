//
// Created by jwscoggins on 12/1/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_HASONBOARDDOTSTARPIXEL_H
#define I960_PROTOTYPE_SIMULATOR_HASONBOARDDOTSTARPIXEL_H
#include <Adafruit_DotStar.h>
template<int dataPin, int clockPin, int numPixels = 1>
class HasOnboardDotStar {
public:
    HasOnboardDotStar() : on(numPixels, dataPin, clockPin) { }
    void begin() {
        on.begin();
    }
    void
    setDotstarPixelValue(uint8_t r, uint8_t g, uint8_t b, bool update = true, int index = 0) {
        on.setPixelColor(index, r,g, b);
        if (update) {
            on.show();
        }
    }
    void
    setDotstarBrightness(uint8_t value) noexcept {
        on.setBrightness(value);
    }
    void
    updateDotStar() noexcept {
        on.show();
    }
private:
    Adafruit_DotStar on;
};

#endif //I960_PROTOTYPE_SIMULATOR_HASONBOARDDOTSTARPIXEL_H

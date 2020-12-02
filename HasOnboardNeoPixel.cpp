//
// Created by jwscoggins on 12/1/20.
//

#include "HasOnboardNeoPixel.h"

HasOnboardNeoPixel::HasOnboardNeoPixel(int pinIndex, int flags) : np(1, pinIndex, flags) { }

void
HasOnboardNeoPixel::begin() {
    np.begin();
    np.show(); // shut off the neo pixel immediately
}

void
HasOnboardNeoPixel::setNeoPixelColor(uint8_t r, uint8_t g, uint8_t b, bool update) noexcept {
    np.setPixelColor(0, r, g, b);
    if (update) {
        np.show();
    }
}
void
HasOnboardNeoPixel::updateNeoPixel() noexcept {
    np.show();
}
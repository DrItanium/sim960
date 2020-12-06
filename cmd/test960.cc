//
// Created by jwscoggins on 12/5/20.
//

#include "Core.h"
#include <iostream>
// custom implementation (this is on purpose)
namespace i960 {
    /*
     * For now, the Grand Central M4 uses an SD Card for its memory with a small portion of the on board sram used for scratchpad / always
     * available memory. On board devices are mapped into the implicit onboard device area of [0xFF00'0000,0xFFFF'FFFF]. The IO device bus
     * exists here as extra "modules" such as:
     * - Clock Generator
     * - RTC
     * - ESP32 Wifi Coprocessor
     * - Display
     * - Sharp Memory Display
     * - GPIOs from the Grand Central
     * - SPI IO Expanders
     * - Speaker
     * - Control Registers
     * - 2 MB Onboard SPI Flash directly mapped into IO Device Space
     * - OPL3 Device
     * - Neopixels
     * - EEPROM
     * - FLASH memory
     * - etc
     */
    Ordinal
    Core::loadOrdinal(Address address) noexcept {
        return 0;
    }

    Integer
    Core::loadInteger(Address address) noexcept {
        return 0;
    }

    ByteOrdinal
    Core::loadByteOrdinal(Address address) noexcept {
        return 0;
    }

    ByteInteger
    Core::loadByteInteger(Address address) noexcept {
        return 0;
    }

    ShortOrdinal
    Core::loadShortOrdinal(Address address) noexcept {
        return 0;
    }

    ShortInteger
    Core::loadShortInteger(Address address) noexcept {
        return 0;
    }

    void
    Core::storeOrdinal(Address address, Ordinal value) noexcept {
    }

    void
    Core::storeByteInteger(Address address, ByteInteger value) {
    }

    void
    Core::storeByteOrdinal(Address address, ByteOrdinal value) noexcept {
    }

    void
    Core::storeShortOrdinal(Address address, ShortOrdinal value) noexcept {
    }

    void
    Core::storeShortInteger(Address address, ShortInteger value) noexcept {
    }

    void
    Core::storeInteger(Address address, Integer value) noexcept {
    }
}

int main() {
    i960::Core testCore(0,4);
    testCore.post();
    testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
    std::cout << "lda 0xfded, r4" << std::endl;
    auto r4 = testCore.getRegister(static_cast<i960::RegisterIndex>(4)).getOrdinal();
    if (r4 == 0xfded) {
        std::cout << "\tLDA successful!" << std::endl;
    } else {
        std::cout << "\tLDA failed!, got " << std::hex << r4 << " instead!" << std::endl;
    }
    //testCore.cycle(0x5c281e02); // mov 2, r5
    return 0;
}
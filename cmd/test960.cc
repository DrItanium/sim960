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

    void
    Core::badInstruction(DecodedInstruction inst) {
        std::cerr << "BAD INSTRUCTION!" << std::endl;
        std::visit([](auto &&value) {
            using K = std::decay_t<decltype(value)>;
            std::cerr << "Instruction opcode: 0x";
            if constexpr (std::is_same_v<K, i960::MEMFormatInstruction>) {
                std::cerr << std::hex << value.upperHalf();
            }
            std::cerr << std::hex << value.lowerHalf() << std::endl;
            if (auto name = value.decodeName(); !name.empty()) {
                std::cerr << "Name: " << name << std::endl;
            }
        }, inst);
        raiseFault();
    }



    void
    test0() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        i960::Core testCore(0,4);
        testCore.post();
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x5c281e02); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'40'04); // addo r4,r5,r6
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    test1() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        i960::Core testCore(0,4);
        testCore.post();
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        // subi r4, r5, r6
        testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x5c281e02); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'40'04); // addo r4,r5,r6
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'41'84); // subi r4,r5,r6
        std::cout << "subi r4, r5, r6" << std::endl;
        if (r6.getInteger() != (2 - 0xFDED)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    test2() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        i960::Core testCore(0,4);
        testCore.post();
        // double check that registers are clear at this point
        auto l4 = static_cast<i960::RegisterIndex>(4);
        auto l5 = static_cast<i960::RegisterIndex>(5);
        auto l6 = static_cast<i960::RegisterIndex>(6);
        auto& r4 = testCore.getRegister(l4);
        if (r4.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        auto& r5 = testCore.getRegister(l5);
        if (r5.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r5!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }
        auto& r6 = testCore.getRegister(l6);
        if (r6.getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r6!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // lda 0xfded, r4
        // mov 2, r5
        // addo r4, r5, r6
        // subi r4, r5, r6
        // subi r5, r4, r6
        testCore.cycle(0x8c203000, 0x0000fded); // LDA 0xfded, r4
        std::cout << "lda 0xfded, r4" << std::endl;
        if (r4.getOrdinal() != 0xfded) {
            std::cout << "\tfailed!, got " << std::hex << r4.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x5c281e02); // mov 2, r5
        std::cout << "mov 2, r5" << std::endl;
        if (r5.getOrdinal() != 2) {
            std::cout << "\tfailed!, got " << std::hex << r5.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'40'04); // addo r4,r5,r6
        std::cout << "addo r4, r5, r6" << std::endl;
        if (r6.getOrdinal() != (0xfded + 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }

        testCore.cycle(0x59'31'41'84); // subi r4,r5,r6
        std::cout << "subi r4, r5, r6" << std::endl;
        if (r6.getInteger() != (2 - 0xFDED)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x59'31'01'85); // subi r5,r4,r6
        std::cout << "subi r5, r4, r6" << std::endl;
        if (r6.getInteger() != (0xFDED - 2)) {
            std::cout << "\tfailed!, got " << std::hex << r6.getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testB() {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        i960::Core testCore(0,4);
        testCore.post();
        // double check that registers are clear at this point
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // b 0x8000 <destination>
        testCore.cycle(0x08'00'80'00); // b
        std::cout << "b 0x8000" << std::endl;
        if (testCore.getIP().getOrdinal() != 0x8000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testCall() {
        // test call
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        i960::Core testCore(0,4);
        testCore.post();
        // double check that registers are clear at this point
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // call 0x9000 <destination>
        testCore.cycle(0x09'00'90'00); // b
        std::cout << "call 0x9000" << std::endl;
        if (testCore.getIP().getOrdinal() != 0x9000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
    }
    void
    testBal() {
        // test bal
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        // make sure that each instruction operates as expected
        i960::Core testCore(0,4);
        testCore.post();
        // double check that registers are clear at this point
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tAssertion Failed on r4!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        // okay now here is the test itself
        // run a simple program:
        // call 0x9000 <destination>
        testCore.cycle(0x0b'00'a0'00); // bal
        std::cout << "bal 0xa000" << std::endl;
        if (testCore.getIP().getOrdinal() != 0xa000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        testCore.cycle(0x0b'ff'60'00); // bal
        std::cout << "bal start" << std::endl;
        if (testCore.getIP().getOrdinal() != 0) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        /// @todo check the frame pointers and such at some point in the future
        // offset testing
        testCore.nextInstruction();
        testCore.cycle(0x0b'00'9f'fc); // bal
        std::cout << "bal 0xa000 // offset style" << std::endl;
        if (testCore.getIP().getOrdinal() != 0xa000) {
            std::cout << "\tfailed!, got " << std::hex << testCore.getIP().getOrdinal() << " instead!" << std::endl;
        }
        std::cout << std::endl;
        /// @todo check the frame pointers and such at some point in the future
    }
}


int main() {
    i960::test0();
    i960::test1();
    i960::test2();
    i960::testB();
    i960::testCall();
    i960::testBal();
    return 0;
}
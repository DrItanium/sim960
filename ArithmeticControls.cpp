//
// Created by jwscoggins on 11/27/20.
//

#include "ArithmeticControls.h"

namespace i960 {
    void
    ArithmeticControls::setConditionCode(ByteOrdinal cc) noexcept {
        value = (value & (~0b111)) | (static_cast<Ordinal>(cc));
    }
    void
    ArithmeticControls::setIntegerOverflowFlag(bool value) noexcept {
        static constexpr Ordinal On = 1u << 8;
        static constexpr Ordinal Mask = ~On;
        auto maskedValue = value & Mask;
        if (value) {
            maskedValue |= On;
        }
        value = maskedValue;
    }
    void
    ArithmeticControls::setIntegerOverflowMask(bool value) noexcept {
        static constexpr Ordinal On = 1u << 12;
        static constexpr Ordinal Mask = ~On;
        auto maskedValue = value & Mask;
        if (value) {
            maskedValue |= On;
        }
        value = maskedValue;
    }
    void
    ArithmeticControls::setNoImpreciseFaults(bool value) noexcept {
        static constexpr Ordinal On = 1u << 15;
        static constexpr Ordinal Mask = ~On;
        auto maskedValue = value & Mask;
        if (value) {
            maskedValue |= On;
        }
        value = maskedValue;
    }
    void
    ArithmeticControls::setCarryFlag(bool value) noexcept {
        auto modCond = getConditionCode() & 0b101;
        if (value) {
            setConditionCode(modCond | 0b010);
        } else {
            setConditionCode(modCond);
        }
    }
    void
    ArithmeticControls::setOverflowFlag(bool value) noexcept {
        auto modCond = getConditionCode() & 0b110;
        if (value) {
            setConditionCode(modCond | 0b001);
        } else {
            setConditionCode(modCond);
        }
    }
    void
    ArithmeticControls::clearConditionCode() noexcept {
        setConditionCode(0);
    }
}


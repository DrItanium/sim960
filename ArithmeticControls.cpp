//
// Created by jwscoggins on 11/27/20.
//

#include "ArithmeticControls.h"

namespace i960 {
    void
    ArithmeticControls::setConditionCode(ByteOrdinal value) noexcept {
        _targetRegister.setOrdinal((_targetRegister.getOrdinal() & (~0b111)) | static_cast<Ordinal>(value));
    }
    void
    ArithmeticControls::setIntegerOverflowFlag(bool value) noexcept {
        static constexpr Ordinal On = 1u << 8;
        static constexpr Ordinal Mask = ~On;
        auto maskedValue = _targetRegister.getOrdinal() & Mask;
        if (value) {
            maskedValue |= On;
        }
        _targetRegister.setOrdinal(maskedValue);
    }
    void
    ArithmeticControls::setIntegerOverflowMask(bool value) noexcept {
        static constexpr Ordinal On = 1u << 12;
        static constexpr Ordinal Mask = ~On;
        auto maskedValue = _targetRegister.getOrdinal() & Mask;
        if (value) {
            maskedValue |= On;
        }
        _targetRegister.setOrdinal(maskedValue);
    }
    void
    ArithmeticControls::setNoImpreciseFaults(bool value) noexcept {
        static constexpr Ordinal On = 1u << 15;
        static constexpr Ordinal Mask = ~On;
        auto maskedValue = _targetRegister.getOrdinal() & Mask;
        if (value) {
            maskedValue |= On;
        }
        _targetRegister.setOrdinal(maskedValue);
    }
}


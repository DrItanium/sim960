//
// Created by jwscoggins on 11/27/20.
//

#include "PreviousFramePointer.h"

namespace i960 {

    void
    PreviousFramePointer::setAddress(Ordinal value) noexcept {
        auto preserved = _targetRegister.getOrdinal() & 0b1111;
        auto maskedValue = value & (~0b1111);
        _targetRegister.setOrdinal(preserved | maskedValue);
    }

    void
    PreviousFramePointer::setPreReturnTraceFlag(bool value) noexcept {
        auto masked = _targetRegister.getOrdinal() & (~0b1000);
        if (value) {
           masked |= 0b1000;
        }
        _targetRegister.setOrdinal(masked);
    }

    void
    PreviousFramePointer::setReturnStatus(ByteOrdinal value) noexcept {
        _targetRegister.setOrdinal((_targetRegister.getOrdinal() & (~0b111))| static_cast<Ordinal>(value & 0b111));
    }
}

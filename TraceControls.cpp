//
// Created by jwscoggins on 12/20/20.
//
#include "TraceControls.h"

namespace i960 {
    Ordinal
    TraceControls::modify(Ordinal mask, Ordinal src2) noexcept {
        auto temp = raw_;
        auto intermediateTemp = (EventFlags & raw_ & mask) | (ModeBits & mask);
        raw_ = (intermediateTemp & src2) | (raw_ & ~intermediateTemp);
        return temp;
    }
}

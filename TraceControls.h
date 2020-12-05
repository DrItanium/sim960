//
// Created by jwscoggins on 12/5/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_TRACECONTROLS_H
#define I960_PROTOTYPE_SIMULATOR_TRACECONTROLS_H
#include "CoreTypes.h"

namespace i960 {
class TraceControls {
public:
    constexpr TraceControls() noexcept : _raw(0) { }
    constexpr auto getRawValue() const noexcept { return _raw; }
    void setRawValue(Ordinal value) noexcept { _raw = value; }
    constexpr auto getInstructionTraceMode() const noexcept { return static_cast<bool>(i); }
    constexpr auto getBranchTraceMode() const noexcept { return static_cast<bool>(b); }
    constexpr auto getCallTraceMode() const noexcept { return static_cast<bool>(c); }
    constexpr auto getReturnTraceMode() const noexcept { return static_cast<bool>(r); }
    constexpr auto getPrereturnTraceMode() const noexcept { return static_cast<bool>(p); }
    constexpr auto getSupervisorTraceMode() const noexcept { return static_cast<bool>(s); }
    constexpr auto getMarkTraceMode() const noexcept { return static_cast<bool>(mk); }
    /// @todo continue here
private:
    union {
        Ordinal _raw;
        union {
            unsigned unused0 : 1;
            unsigned i : 1;
            unsigned b : 1;
            unsigned c : 1;
            unsigned r : 1;
            unsigned p : 1;
            unsigned s : 1;
            unsigned mk : 1;
            /// @todo implement the hardware breakpoint event flags, unsure how to read them right now
        };
    };
};
}

#endif //I960_PROTOTYPE_SIMULATOR_TRACECONTROLS_H

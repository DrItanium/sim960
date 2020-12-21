//
// Created by jwscoggins on 12/5/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_TRACECONTROLS_H
#define I960_PROTOTYPE_SIMULATOR_TRACECONTROLS_H
#include "CoreTypes.h"

namespace i960 {
class TraceControls {
public:
    static constexpr Ordinal ModeBits = 0x0000'00FE;
    static constexpr Ordinal EventFlags = 0x0FFEFF00;
public:
    constexpr TraceControls() noexcept : raw_(0) { }
    [[nodiscard]] constexpr auto getRawValue() const noexcept { return raw_; }
    void setRawValue(Ordinal value) noexcept { raw_ = value; }
    [[nodiscard]] constexpr auto getInstructionTraceMode() const noexcept { return static_cast<bool>(i); }
    [[nodiscard]] constexpr auto getBranchTraceMode() const noexcept { return static_cast<bool>(b); }
    [[nodiscard]] constexpr auto getCallTraceMode() const noexcept { return static_cast<bool>(c); }
    [[nodiscard]] constexpr auto getReturnTraceMode() const noexcept { return static_cast<bool>(r); }
    [[nodiscard]] constexpr auto getPrereturnTraceMode() const noexcept { return static_cast<bool>(p); }
    [[nodiscard]] constexpr auto getSupervisorTraceMode() const noexcept { return static_cast<bool>(s); }
    [[nodiscard]] constexpr auto getMarkTraceMode() const noexcept { return static_cast<bool>(mk); }
    [[nodiscard]] Ordinal modify(Ordinal mask, Ordinal src2) noexcept;

    /// @todo continue here
private:
    union {
        Ordinal raw_;
        struct {
            unsigned unused0_ : 1;
            /// instruction trace mode
            unsigned i : 1;
            /// branch trace mode
            unsigned b : 1;
            /// call trace mode
            unsigned c : 1;
            /// Return trace mode
            unsigned r : 1;
            /// Prereturn trace mode
            unsigned p : 1;
            /// Supervisor Trace Mode
            unsigned s : 1;
            /// Mark Trace Mode
            unsigned mk : 1;
            /// Instruction-Address Breakpoint 2 to 5
            unsigned i25f : 4;
            /// Data-Address Breakpoint 2 to 5
            unsigned d25f : 4;
            unsigned unused1_: 1;
            /// Instruction Trace Event
            unsigned ite: 1;
            /// Branch Trace Event
            unsigned bte: 1;
            /// Call Trace Event
            unsigned cte: 1;
            /// Return Trace Event
            unsigned rte: 1;
            /// Prereturn Trace Event
            unsigned prte: 1;
            /// Supervisor Trace Event
            unsigned sute: 1;
            /// Breakpoint Trace Event
            unsigned bpte: 1;
            /// Instruction-Address Breakpoint 0 to 1
            unsigned iab01 : 2;
            /// Data-Address Breakpoint 0 to 1
            unsigned dab01 : 2;
            unsigned unused2_ : 4;
        };
    };
};
static_assert(sizeof(TraceControls) == sizeof(Ordinal));
}

#endif //I960_PROTOTYPE_SIMULATOR_TRACECONTROLS_H

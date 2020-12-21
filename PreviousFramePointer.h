//
// Created by jwscoggins on 11/27/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_PREVIOUSFRAMEPOINTER_H
#define I960_PROTOTYPE_SIMULATOR_PREVIOUSFRAMEPOINTER_H
#include "Register.h"
namespace i960 {
    class PreviousFramePointer {
    public:
        enum class ReturnStatusField : ByteOrdinal {
            Local = 0b000,
            Fault = 0b001,
            Supervisor_TraceDisabled = 0b010,
            Supervisor_TraceEnabled = 0b011,
            Reserved0 = 0b100,
            Reserved1 = 0b101,
            Reserved2 = 0b110,
            Interrupt = 0b111,
        };
    public:
        explicit PreviousFramePointer(Register &target) : targetRegister_(target) {}
        constexpr ReturnStatusField getReturnType() const noexcept { return static_cast<ReturnStatusField>(targetRegister_.getByteOrdinal() & 0b111); }
        constexpr bool getPreReturnTraceFlag() const noexcept { return targetRegister_.getByteOrdinal() & 0b1000; }
        constexpr Ordinal getAddress() const noexcept { return targetRegister_.getOrdinal() & (~0b1111); }
        void setAddress(Ordinal value) noexcept { targetRegister_.setOrdinal(value & (~0b1111)); }
        void setPreReturnTraceFlag(bool value) noexcept {
            targetRegister_.setOrdinal((targetRegister_.getOrdinal() & (~0b1000)) | (value ? 0b1000 : 0b0000));
        }
        void setReturnStatus(ReturnStatusField value) noexcept {
            auto masked = targetRegister_.getOrdinal() & ~0b111;
            targetRegister_.setOrdinal(masked | (static_cast<ByteOrdinal>(value) & 0b111));
        }
        void setRawValue(Ordinal value) noexcept {
            targetRegister_.setOrdinal(value);
        }
    private:
        Register& targetRegister_;
    };
}

#endif //I960_PROTOTYPE_SIMULATOR_PREVIOUSFRAMEPOINTER_H

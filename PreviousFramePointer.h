//
// Created by jwscoggins on 11/27/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_PREVIOUSFRAMEPOINTER_H
#define I960_PROTOTYPE_SIMULATOR_PREVIOUSFRAMEPOINTER_H
#include "Register.h"
namespace i960 {
    class PreviousFramePointer {
    public:
        explicit PreviousFramePointer(Register &target) : _targetRegister(target) {}
        constexpr ByteOrdinal getReturnType() const noexcept { return _targetRegister.getByteOrdinal() & 0b111; }
        constexpr bool getPreReturnTraceFlag() const noexcept { return _targetRegister.getByteOrdinal() & 0b1000; }
        constexpr Ordinal getAddress() const noexcept { return _targetRegister.getOrdinal() & (~0b1111); }
        void setAddress(Ordinal value) noexcept;
        void setPreReturnTraceFlag(bool value) noexcept;
        void setReturnStatus(ByteOrdinal value) noexcept;
    private:
        Register &_targetRegister;
    };
}

#endif //I960_PROTOTYPE_SIMULATOR_PREVIOUSFRAMEPOINTER_H

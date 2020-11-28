//
// Created by jwscoggins on 11/27/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H
#define I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H
#include "Register.h"
#include "CoreTypes.h"
namespace i960 {
    class ArithmeticControls {
    public:
        ArithmeticControls(Register& target) : _targetRegister(target) { }
        constexpr ByteOrdinal getConditionCode() const noexcept { return _targetRegister.getByteOrdinal() & 0b111; }
        constexpr bool getIntegerOverflowFlag() const noexcept { return (_targetRegister.getOrdinal() >> 8) & 1; }
        constexpr bool getIntegerOverflowMask() const noexcept { return (_targetRegister.getOrdinal() >> 12) & 1; }
        constexpr bool getNoImpreciseFaults() const noexcept { return (_targetRegister.getOrdinal() >> 15) & 1; }
        void setConditionCode(ByteOrdinal value) noexcept;
        void setIntegerOverflowFlag(bool value) noexcept;
        void setIntegerOverflowMask(bool value) noexcept;
        void setNoImpreciseFaults(bool value) noexcept;
    private:
        Register& _targetRegister;
    };
} // end namespace i960
#endif //I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H

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
        constexpr ByteOrdinal getConditionCode() const noexcept { return static_cast<ByteOrdinal>(value) & 0b111; }
        constexpr bool getIntegerOverflowFlag() const noexcept { return (value >> 8) & 1; }
        constexpr bool getIntegerOverflowMask() const noexcept { return (value >> 12) & 1; }
        constexpr bool getNoImpreciseFaults() const noexcept { return (value >> 15) & 1; }
        void setConditionCode(ByteOrdinal value) noexcept;
        void setIntegerOverflowFlag(bool value) noexcept;
        void setIntegerOverflowMask(bool value) noexcept;
        void setNoImpreciseFaults(bool value) noexcept;
        void setRawValue(Ordinal raw) noexcept { value = raw; }
        constexpr Ordinal getRawValue() const noexcept { return value; }
        constexpr bool carryFlagSet() const noexcept { return (getConditionCode() & 0b010); }
        constexpr bool overflowSet() const noexcept { return (getConditionCode() & 1); }
        void setCarryFlag(bool value) noexcept;
        void setOverflowFlag(bool value) noexcept;
        void clearConditionCode() noexcept;

        constexpr bool conditionIsUnordered() const noexcept { return getConditionCode() == 0b000; }
        constexpr bool conditionIsGreaterThan() const noexcept { return getConditionCode() & 0b001; }
        constexpr bool conditionIsEqualTo() const noexcept { return getConditionCode() & 0b010; }
        constexpr bool conditionIsGreaterThanOrEqualTo() const noexcept { return getConditionCode() & 0b011; }
        constexpr bool conditionIsLessThan() const noexcept { return getConditionCode() & 0b100; }
        constexpr bool conditionIsNotEqual() const noexcept { return getConditionCode() & 0b101; }
        constexpr bool conditionIsLessThanOrEqual() const noexcept { return getConditionCode() & 0b110; }
        constexpr bool conditionIsOrdered() const noexcept { return static_cast<bool>(getConditionCode()); }
    private:
        Ordinal value = 0;
    };
} // end namespace i960
#endif //I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H

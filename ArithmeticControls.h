//
// Created by jwscoggins on 11/27/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H
#define I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H
#include "Register.h"
#include "CoreTypes.h"
namespace i960 {
    enum class ConditionCodeKind : ByteOrdinal {
        Unordered = 0b000,
        GreaterThan = 0b001,
        EqualTo = 0b010,
        GreaterThanOrEqualTo = 0b011,
        LessThan = 0b100,
        NotEqual = 0b101,
        LessThanOrEqual = 0b110,
        Ordered = 0b111
    };
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

        template<ConditionCodeKind cck>
        constexpr bool conditionIs() const noexcept {
            if constexpr (cck == ConditionCodeKind::Unordered) {
                return getConditionCode() == 0;
            } else if constexpr (cck == ConditionCodeKind::Ordered) {
                return static_cast<bool>(getConditionCode());
            } else {
                return getConditionCode() & static_cast<std::underlying_enum_type_t<ConditionCodeKind>>(cck);
            }
        }
        constexpr bool conditionIsUnordered() const noexcept { return conditionIs<ConditionCodeKind::Unordered>(); }
        constexpr bool conditionIsGreaterThan() const noexcept { return conditionIs<ConditionCodeKind::GreaterThan>(); }
        constexpr bool conditionIsEqualTo() const noexcept { return conditionIs<ConditionCodeKind::EqualTo>(); }
        constexpr bool conditionIsGreaterThanOrEqualTo() const noexcept { return conditionIs<ConditionCodeKind::GreaterThanOrEqualTo>(); }
        constexpr bool conditionIsLessThan() const noexcept { return conditionIs<ConditionCodeKind::LessThan>(); }
        constexpr bool conditionIsNotEqual() const noexcept { return conditionIs<ConditionCodeKind::NotEqual>(); }
        constexpr bool conditionIsLessThanOrEqual() const noexcept { return conditionIs<ConditionCodeKind::LessThanOrEqual>(); }
        constexpr bool conditionIsOrdered() const noexcept { return conditionIs<ConditionCodeKind::Ordered>(); }
    private:
        Ordinal value = 0;
    };
} // end namespace i960
#endif //I960_PROTOTYPE_SIMULATOR_ARITHMETICCONTROLS_H

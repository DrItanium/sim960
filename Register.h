//
// Created by jwscoggins on 11/27/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_REGISTER_H
#define I960_PROTOTYPE_SIMULATOR_REGISTER_H
#include <variant>
#include <string>
#include "CoreTypes.h"
namespace i960 {
    /**
     * @brief A value between 0..31
     */
    enum class Literal : uint8_t { };
    constexpr std::underlying_type_t<Literal> toInteger(Literal value) noexcept {
        auto converted = static_cast<std::underlying_type_t<Literal>>(value);
        // from 0..31 so chop the values
        return converted & 0b11111;
    }
    constexpr Literal toLiteral(int value) noexcept {
        auto chopped = static_cast<std::underlying_type_t<Literal>>(value);
        // make sure we are in the range 0...31
        return static_cast<Literal>(chopped & 0b11111);
    }

    /**
     * @brief A register index
     */
    enum class RegisterIndex : uint8_t { };
    constexpr std::underlying_type_t<RegisterIndex> toInteger(RegisterIndex value) noexcept {
        auto converted = static_cast<std::underlying_type_t<RegisterIndex>>(value);
        // from 0..31 so chop the values
        return converted & 0b11111;
    }
    constexpr RegisterIndex toRegisterIndex(int value) noexcept {
        auto chopped = static_cast<std::underlying_type_t<RegisterIndex>>(value);
        // make sure we are in the range 0...31
        return static_cast<RegisterIndex>(chopped & 0b11111);
    }
    constexpr RegisterIndex nextRegisterIndex(RegisterIndex value) noexcept {
        return toRegisterIndex(toInteger(value)  + 1);
    }
    constexpr bool divisibleByTwo(RegisterIndex value) noexcept { return (toInteger(value) & 0b1) == 0; }
    constexpr bool divisibleByFour(RegisterIndex value) noexcept { return (toInteger(value) & 0b11) == 0; }

    class Displacement22 {
    public:
        constexpr Displacement22(int32_t disp) noexcept : value(disp) { }
        constexpr auto getValue() const noexcept { return value; }
    private:
        int32_t value : 22;
    };
    class AbsoluteOffset {
    public:
        constexpr AbsoluteOffset(int32_t offset) noexcept : value(offset) { }
        constexpr auto getValue() const noexcept { return value; }
    private:
        int32_t value;
    };
    class RegisterIndirect {
    public:
        constexpr RegisterIndirect(RegisterIndex abase) noexcept : idx(abase) { }
        constexpr auto getAbase() const noexcept { return idx; }
    private:
        RegisterIndex idx;
    };
    /// @todo add classes to describe the other memory modes
    using RegLit = std::variant<RegisterIndex, Literal>;
    constexpr bool isRegisterIndex(RegLit value) noexcept { return std::holds_alternative<RegisterIndex>(value); }
    constexpr bool isLiteral(RegLit value) noexcept { return std::holds_alternative<Literal>(value); }
    class Register {
    public:
        constexpr Register() : ordValue(0) { }
        [[nodiscard]] constexpr Ordinal getOrdinal() const noexcept { return ordValue; }
        [[nodiscard]] constexpr Integer getInteger() const noexcept { return intValue; }
        [[nodiscard]] constexpr auto getShortOrdinal(bool upper = false) const noexcept { return shortOrds[upper ? 1 : 0 ]; }
        [[nodiscard]] constexpr auto getShortInteger(bool upper = false) const noexcept { return shortInts[upper ? 1 : 0 ]; }
        [[nodiscard]] constexpr auto getByteOrdinal(int index = 0) const noexcept { return byteOrds[index & 0b11]; }
        [[nodiscard]] constexpr auto getByteInteger(int index = 0) const noexcept { return byteInts[index & 0b11]; }
        void setOrdinal(Ordinal value) noexcept { ordValue = value; }
        void setInteger(Integer value) noexcept { intValue = value; }
        void setShortOrdinal(ShortOrdinal value, bool upper = false) noexcept { shortOrds[upper ? 1 : 0] = value; }
        void setShortInteger(ShortInteger value, bool upper = false) noexcept { shortInts[upper ? 1 : 0] = value; }
        void setByteOrdinal(ByteOrdinal value, int index = 0) noexcept { byteOrds[index & 0b11] = value; }
        void setByteInteger(ByteInteger value, int index = 0) noexcept { byteInts[index & 0b11] = value; }
        void set(Ordinal value, TreatAsOrdinal) noexcept { setOrdinal(value); }
        void set(Integer value, TreatAsInteger) noexcept { setInteger(value); }
        explicit operator Ordinal() const noexcept { return getOrdinal(); }
        explicit operator Integer() const noexcept { return getInteger(); }
        void increment(Ordinal value) noexcept { ordValue += value; }
    private:
        union {
            Ordinal ordValue;
            Integer intValue;
            ShortOrdinal shortOrds[2];
            ShortInteger shortInts[2];
            ByteOrdinal byteOrds[4];
            ByteInteger byteInts[4];
        };
    };
    class LongRegister {
    public:
        LongRegister(Register& lower, Register& upper) noexcept : _lower(lower), _upper(upper) { }
        void setOrdinal(LongOrdinal value) noexcept {
            _lower.setOrdinal(static_cast<Ordinal>(value));
            _upper.setOrdinal(static_cast<Ordinal>(value >> 32));
        }
        void setInteger(LongInteger value) noexcept {
            _lower.setInteger(static_cast<Integer>(value));
            _upper.setInteger(static_cast<Integer>(value >> 32));
        }
        constexpr LongOrdinal getOrdinal() const noexcept {
            auto lword = static_cast<LongOrdinal>(_lower.getOrdinal());
            auto uword = static_cast<LongOrdinal>(_upper.getOrdinal()) << 32;
            return lword | uword;
        }
        constexpr LongInteger getInteger() const noexcept {
            auto lword = static_cast<LongInteger>(_lower.getInteger());
            auto uword = static_cast<LongInteger>(_upper.getInteger()) << 32;
            return lword | uword;
        }
        explicit operator LongOrdinal() const noexcept { return getOrdinal(); }
        explicit operator LongInteger() const noexcept { return getInteger(); }
    private:
        Register& _lower;
        Register& _upper;
    };
} // end namespace i960
#endif //I960_PROTOTYPE_SIMULATOR_REGISTER_H

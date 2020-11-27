#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include "CoreTypes.h"
#include "TargetPlatform.h"
#include "MemoryInterface.h"
#include "DependentFalse.h"
#include <variant>
namespace i960
{

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

    /**
     * @brief A register index
     */
    class Displacement {
    public:
        constexpr Displacement(int32_t disp) noexcept : value(disp) { }
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

    using MemoryAddressing = std::variant<AbsoluteOffset, RegisterIndirect>; // continue to add new targets here
    using RegLit = std::variant<RegisterIndex, Literal>;
    constexpr bool isRegisterIndex(RegLit value) noexcept { return std::holds_alternative<RegisterIndex>(value); }
    constexpr bool isLiteral(RegLit value) noexcept { return std::holds_alternative<Literal>(value); }
    class Register {
    public:
        constexpr Register() : ordValue(0) { }
        constexpr Ordinal getOrdinal() const noexcept { return ordValue; }
        constexpr Integer getInteger() const noexcept { return intValue; }
        constexpr auto getShortOrdinal(bool upper = false) const noexcept { return shortOrds[upper ? 1 : 0 ]; }
        constexpr auto getShortInteger(bool upper = false) const noexcept { return shortInts[upper ? 1 : 0 ]; }
        constexpr auto getByteOrdinal(int index = 0) const noexcept { return byteOrds[index & 0b11]; }
        constexpr auto getByteInteger(int index = 0) const noexcept { return byteInts[index & 0b11]; }
        void setOrdinal(Ordinal value) noexcept { ordValue = value; }
        void setInteger(Integer value) noexcept { intValue = value; }
        void setShortOrdinal(ShortOrdinal value, bool upper = false) noexcept { shortOrds[upper ? 1 : 0] = value; }
        void setShortInteger(ShortInteger value, bool upper = false) noexcept { shortInts[upper ? 1 : 0] = value; }
        void setByteOrdinal(ByteOrdinal value, int index = 0) noexcept { byteOrds[index & 0b11] = value; }
        void setByteInteger(ByteInteger value, int index = 0) noexcept { byteInts[index & 0b11] = value; }
        void increment() noexcept { ++ordValue; }
        void decrement() noexcept { --ordValue; }
        explicit operator Ordinal() const noexcept { return getOrdinal(); }
        explicit operator Integer() const noexcept { return getInteger(); }
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

    using RegisterFile = std::array<Register, 16>;
    class Core {
    public:
        Core(MemoryInterface& mi);
        void cycle();
    private: // common internal functions
        Register& getRegister(int index) noexcept;
        inline Register& getRegister(RegisterIndex index) noexcept { return getRegister(toInteger(index)); }
        const Register& getRegister(int index) const noexcept;
        inline const Register& getRegister(RegisterIndex index) const noexcept { return getRegister(toInteger(index)); }
        void moveRegisterContents(int from, int to) noexcept;
        void moveRegisterContents(RegisterIndex from, RegisterIndex to) noexcept { moveRegisterContents(toInteger(from), toInteger(to)); }
        void saveLocals() noexcept;
        void restoreLocals() noexcept;
        bool getCarryFlag() const noexcept;
        void setCarryFlag(bool value) noexcept;
    private: // memory controller interface routines for abstraction purposes
        Ordinal loadOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsOrdinal{}); }
        void storeOrdinal (Address address, Ordinal value) noexcept { memoryController.storeValue(address, value, TreatAsOrdinal{}); }
        Integer loadInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsInteger{}); }
        void storeInteger (Address address, Integer value) noexcept { memoryController.storeValue(address, value, TreatAsInteger{}); }

        ByteOrdinal loadByteOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsByteOrdinal{}); }
        void storeByteOrdinal (Address address, ByteOrdinal value) noexcept { memoryController.storeValue(address, value, TreatAsByteOrdinal{}); }
        ByteInteger loadByteInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsByteInteger{}); }
        void storeByteInteger (Address address, ByteInteger value) noexcept { memoryController.storeValue(address, value, TreatAsByteInteger{}); }

        ShortOrdinal loadShortOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsShortOrdinal{}); }
        void storeShortOrdinal (Address address, ShortOrdinal value) noexcept { memoryController.storeValue(address, value, TreatAsShortOrdinal{}); }
        ShortInteger loadShortInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsShortInteger{}); }
        void storeShortInteger (Address address, ShortInteger value) noexcept { memoryController.storeValue(address, value, TreatAsShortInteger{}); }
    private: // data movement operations
        // mem reg {
        void ld(MemoryAddressing mem, RegisterIndex dest);
        void ldob(MemoryAddressing mem, RegisterIndex dest);
        void ldos(MemoryAddressing mem, RegisterIndex dest);
        void ldib(MemoryAddressing mem, RegisterIndex dest);
        void ldis(MemoryAddressing mem, RegisterIndex dest);
        void ldl(MemoryAddressing mem, RegisterIndex dest);
        void ldt(MemoryAddressing mem, RegisterIndex dest);
        void ldq(MemoryAddressing mem, RegisterIndex dest);

        void st(MemoryAddressing mem, RegisterIndex dest);
        void stob(MemoryAddressing mem, RegisterIndex dest);
        void stos(MemoryAddressing mem, RegisterIndex dest);
        void stib(MemoryAddressing mem, RegisterIndex dest);
        void stis(MemoryAddressing mem, RegisterIndex dest);
        void stl(MemoryAddressing mem, RegisterIndex dest);
        void stt(MemoryAddressing mem, RegisterIndex dest);
        void stq(MemoryAddressing mem, RegisterIndex dest);

        void lda(MemoryAddressing mem, RegisterIndex dest); // efa is another accepted value
        // }

        void mov(RegLit src, RegisterIndex dest);
        void movl(RegLit src, RegisterIndex dest);
        void movt(RegLit src, RegisterIndex dest);
        void movq(RegLit src, RegisterIndex dest);

    private: // arithmetic
        Ordinal extractValue(RegLit value, TreatAsOrdinal) const noexcept {
            return std::visit([this](auto&& value) -> Ordinal {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    return static_cast<Ordinal>(toInteger(value));
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return getRegister(value).getOrdinal();
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                    return 0;
                }
            }, value);
        }
        Integer extractValue(RegLit value, TreatAsInteger) const noexcept {
            return std::visit([this](auto&& value) -> Integer{
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    return static_cast<Integer>(toInteger(value));
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return getRegister(value).getInteger();
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                    return 0;
                }
            }, value);
        }
        RegLit nextValue(RegLit value) const noexcept {
            return std::visit([this](auto&& value) -> RegLit {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    // in this case it should always be zero
                    return toLiteral(0);
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return nextRegisterIndex(value);
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                }
            }, value);
        }
        /// @todo figure out the different code forms
        void addi(RegLit src1, RegLit src2, RegisterIndex dest);
        void addo(RegLit src1, RegLit src2, RegisterIndex dest);
        void addc(RegLit src1, RegLit src2, RegisterIndex dest);
        void subi(RegLit src1, RegLit src2, RegisterIndex dest);
        void subo(RegLit src1, RegLit src2, RegisterIndex dest);
        void subc(RegLit src1, RegLit src2, RegisterIndex dest);
        void muli(RegLit src1, RegLit src2, RegisterIndex dest);
        void mulo(RegLit src1, RegLit src2, RegisterIndex dest);
        void divi(RegLit src1, RegLit src2, RegisterIndex dest);
        void divo(RegLit src1, RegLit src2, RegisterIndex dest);
        void emul(RegLit src1, RegLit src2, RegisterIndex dest);
        void ediv(RegLit src1, RegLit src2, RegisterIndex dest);
        void remi(RegLit src1, RegLit src2, RegisterIndex dest);
        void remo(RegLit src1, RegLit src2, RegisterIndex dest);
        void modi(RegLit src1, RegLit src2, RegisterIndex dest);
        void shlo(RegLit src1, RegLit src2, RegisterIndex dest);
        void shli(RegLit src1, RegLit src2, RegisterIndex dest);
        void shro(RegLit src1, RegLit src2, RegisterIndex dest);
        void shri(RegLit src1, RegLit src2, RegisterIndex dest);
        void shrdi(RegLit src1, RegLit src2, RegisterIndex dest);
        void rotate(RegLit len, RegLit src, RegisterIndex dest);

    private: // logical operations
        void logicalAnd(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalOr(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalXor(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalNand(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalNor(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalXnor(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalNot(RegLit src, RegisterIndex dest);
        void notand(RegLit src1, RegLit src2, RegisterIndex dest);
        void andnot(RegLit src1, RegLit src2, RegisterIndex dest);
        void notor(RegLit src1, RegLit src2, RegisterIndex dest);
        void ornot(RegLit src1, RegLit src2, RegisterIndex dest);
    private: // bit and bit-field operations
        void setbit(RegLit bitpos, RegLit src, RegisterIndex dest);
        void clrbit(RegLit bitpos, RegLit src, RegisterIndex dest);
        void notbit(RegLit src1, RegLit src2, RegisterIndex dest);
        void chkbit(RegLit bitpos, RegLit src);
        void alterbit(RegLit bitpos, RegLit src, RegisterIndex dest);
        void scanbit(RegLit src, RegisterIndex dest);
        void spanbit(RegLit src, RegisterIndex dest);
        void extract(RegLit bitpos, RegLit len, RegisterIndex srcDest);
        void modify(RegLit bitpos, RegLit len, RegisterIndex srcDest);
        void scanbyte(RegLit src1, RegLit src2);
    private: // compare and increment or decrement
        void concmpi(RegLit src1, RegLit src2);
        void concmpo(RegLit src1, RegLit src2);
        void cmpinci(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpinco(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpi(RegLit src1, RegLit src2);
        void cmpo(RegLit src1, RegLit src2);
    private: // branching
        /// @todo figure out correct signatures
        void b(Displacement targ);
        void bx(MemoryAddressing targ);
        void bal(Displacement targ);
        void balx(MemoryAddressing targ, RegisterIndex dest); // mem, reg

        /// @todo figure out correct signatures
        void be(Displacement dest);
        void bne(Displacement dest);
        void bl(Displacement dest);
        void ble(Displacement dest);
        void bg(Displacement dest);
        void bge(Displacement dest);
    private: // compare and branch
        /// @todo figure out correct signatures
        void cmpibe(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpobe(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpibne(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpobne(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpibl(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpobl(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpible(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpoble(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpibg(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpobg(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpibge(RegLit src1, RegisterIndex src2, Displacement targ);
        void cmpobge(RegLit src1, RegisterIndex src2, Displacement targ);
        void bbs(RegLit bitpos, RegisterIndex src, Displacement targ);
        void bbc(RegLit bitpos, RegisterIndex src, Displacement targ);
    private: // test condition codes
        void teste(RegisterIndex dest);
        void testne(RegisterIndex dest);
        void testl(RegisterIndex dest);
        void testle(RegisterIndex dest);
        void testg(RegisterIndex dest);
        void testge(RegisterIndex dest);
    private: // call and return (note, no supervisor mode right now)
        /// @todo figure out correct signatures
        void call(Displacement targ);
        void callx(MemoryAddressing targ); // mem
        void ret();
        /// @todo implement faults as exceptions
    private: // processor management
        void flushreg(); // noop right now
        void modac(RegLit mask, RegLit src, RegisterIndex dest);
    private: // Numerics Architecture addons
        void dmovt(RegisterIndex src, RegisterIndex dest);
        void dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest);
        /**
         * @brief Decimal Add with Carry. Adds bits 0 through 3 of src2 and src1 and bit 1 of the condition code (used here as a carry bit).
         * The result is stored in bits 0 through 3 of dest. If the addition results in a carry, bit 1 of the condition code is set. Bits 4
         * through 31 of src2 are copied to dst unchanged.
         *
         * The instruction is intended to be used iteratively to add binary-coded-decimal (BCD) values in which the least-significant four bits
         * of the operands represent the decimal numbers 0 to 9. The instruction assumes that the least significant 4 bits of both operands
         * are valid BCD numbers. If these bits are not valid BCD numbers, the resulting value in dest is unpredictable.
         * @param src1 The first bcd number
         * @param src2 The second bcd number
         * @param dest The destination register to store the result in
         */
        void daddc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest);
    private:
        void loadRegister(Address address, RegisterIndex index, TreatAsOrdinal) noexcept;
        void loadRegister(Address address, RegisterIndex index, TreatAsInteger) noexcept;
        void loadRegister(Address address, RegisterIndex index, TreatAsByteInteger) noexcept;
        void loadRegister(Address address, RegisterIndex index, TreatAsByteOrdinal) noexcept;
        void loadRegister(Address address, RegisterIndex index, TreatAsShortInteger) noexcept;
        void loadRegister(Address address, RegisterIndex index, TreatAsShortOrdinal) noexcept;
        void storeRegister(Address address, RegisterIndex index, TreatAsOrdinal) noexcept;
        void storeRegister(Address address, RegisterIndex index, TreatAsInteger) noexcept;
        void storeRegister(Address address, RegisterIndex index, TreatAsByteInteger) noexcept;
        void storeRegister(Address address, RegisterIndex index, TreatAsByteOrdinal) noexcept;
        void storeRegister(Address address, RegisterIndex index, TreatAsShortInteger) noexcept;
        void storeRegister(Address address, RegisterIndex index, TreatAsShortOrdinal) noexcept;
        void storeLongRegister(Address address, RegisterIndex baseIndex) noexcept;
        void loadLongRegister(Address address, RegisterIndex baseIndex) noexcept;
        void storeTripleRegister(Address address, RegisterIndex baseIndex) noexcept;
        void loadTripleRegister(Address address, RegisterIndex baseIndex) noexcept;
        void storeQuadRegister(Address address, RegisterIndex baseIndex) noexcept;
        void loadQuadRegister(Address address, RegisterIndex baseIndex) noexcept;
    private:
        MemoryInterface& memoryController;
        RegisterFile globals, locals;
        Register ip; // always start at address zero
    };
}
#endif // end I960_CORE_H__

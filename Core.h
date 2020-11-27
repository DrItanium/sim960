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

    /**
     * @brief A register index
     */
    class Displacement {
    public:
        Displacement(int32_t disp) : value(disp) { }
        constexpr auto getValue() const noexcept { return value; }
    private:
        int32_t value : 22;
    };

    using InstructionArgument = std::variant<std::monostate, RegisterIndex, Literal, Displacement>;
    template<typename T> constexpr T add(T a, T b) noexcept { return a + b; }
    template<typename T> constexpr T subtract(T a, T b) noexcept { return a - b; }
    template<typename T> constexpr T multiply(T a, T b) noexcept { return a * b; }
    template<typename T> constexpr T divide(T a, T b) noexcept { return a / b; }
    template<typename T> constexpr T remainder(T a, T b) noexcept { return a % b; }
    template<typename T> constexpr T bitwiseOr(T a, T b) noexcept { return a | b; }
    template<typename T> constexpr T bitwiseAnd(T a, T b) noexcept { return a & b; }
    template<typename T> constexpr T bitwiseNot(T a) noexcept { return ~a; }
    template<typename T> constexpr T bitwiseXor(T a, T b) noexcept { return a ^ b; }
    template<typename T> constexpr T bitwiseXnor(T a, T b) noexcept { return bitwiseNot(bitwiseXor(a, b)); }

    // taken from the i960 manual
    template<typename T> constexpr T bitwiseNand(T src1, T src2) noexcept { return bitwiseOr(bitwiseNot(src2), bitwiseNot(src1)); }
    template<typename T> constexpr T bitwiseNor(T src1, T src2) noexcept { return bitwiseAnd(bitwiseNot(src2), bitwiseNot(src1)); }
    template<typename T> constexpr T bitwiseNotAnd(T src1, T src2) noexcept { return bitwiseAnd(bitwiseNot(src2), src1); }
    template<typename T> constexpr T bitwiseNotOr(T src1, T src2) noexcept { return bitwiseOr(bitwiseNot(src2), src1); }
    template<typename T> constexpr T bitwiseOrNot(T src1, T src2) noexcept { return bitwiseOr(src2, bitwiseNot(src1)); }
    template<typename T> constexpr T bitwiseAndNot(T src1, T src2) noexcept { return bitwiseAnd(src2, bitwiseNot(src1)); }
    static_assert(bitwiseNand<uint8_t>(0x1, 0x2) == 0xFF);
    static_assert(bitwiseNand<uint8_t>(0x1, 0x1) == 0xFE);
    static_assert(bitwiseNand<uint8_t>(0x1, 0x3) == static_cast<uint8_t>((~(0x1 & 0x3))));
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
    private:
        Register& getRegister(int index) noexcept;
        inline Register& getRegister(RegisterIndex index) noexcept { return getRegister(toInteger(index)); }
        const Register& getRegister(int index) const noexcept;
        inline const Register& getRegister(RegisterIndex index) const noexcept { return getRegister(toInteger(index)); }
        void moveRegisterContents(int from, int to) noexcept;
        void moveRegisterContents(RegisterIndex from, RegisterIndex to) noexcept { moveRegisterContents(toInteger(from), toInteger(to)); }
        void saveLocals() noexcept;
        void restoreLocals() noexcept;
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
        void ld(int dest, int src, Integer offset);
        void ldob(int dest, int src, Integer offset);
        void ldos(int dest, int src, Integer offset);
        void ldib(int dest, int src, Integer offset);
        void ldis(int dest, int src, Integer offset);
        void ldl(int dest, int src, Integer offset);
        void ldt(int dest, int src, Integer offset);
        void ldq(int dest, int src, Integer offset);

        void st(int dest, int src, Integer offset);
        void stob(int dest, int src, Integer offset);
        void stib(int dest, int src, Integer offset);
        void stos(int dest, int src, Integer offset);
        void stis(int dest, int src, Integer offset);
        void stl(int dest, int src, Integer offset);
        void stt(int dest, int src, Integer offset);
        void stq(int dest, int src, Integer offset);

        void mov(RegLit src, RegisterIndex dest);
        void movl(RegLit src, RegisterIndex dest);
        void movt(RegLit src, RegisterIndex dest);
        void movq(RegLit src, RegisterIndex dest);

        void lda(int dest, int src, Integer offset);
    private: // arithmetic
        using RegLit = std::variant<RegisterIndex, Literal>;
        static constexpr Ordinal extractValue(RegLit value, TreatAsOrdinal) noexcept {
            return std::visit([this](auto&& value) {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    return static_cast<Ordinal>(toInteger(value));
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return getRegister(value).getOrdinal();
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                }
            }
        }
        static constexpr Integer extractValue(RegLit value, TreatAsInteger) noexcept {
            return std::visit([this](auto&& value) {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, Literal>) {
                    return static_cast<Integer>(toInteger(value));
                } else if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return getRegister(value).getInteger();
                } else {
                    static_assert(DependentFalse<K>, "Unimplemented type!");
                }
            }
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
        void cmpinci(int dest, int src0, int src1);
        void cmpinco(int dest, int src0, int src1);
        void cmpdeci(int dest, int src0, int src1);
        void cmpdeco(int dest, int src0, int src1);
    private: // branching
        /// @todo figure out correct signatures
        void b(Integer dest);
        void bx(Integer dest);
        void bal(Integer dest);
        void balx(Integer dest);

        /// @todo figure out correct signatures
        void be(Integer dest);
        void bne(Integer dest);
        void bl(Integer dest);
        void ble(Integer dest);
        void bg(Integer dest);
        void bge(Integer dest);
    private: // compare and branch
        /// @todo figure out correct signatures
        void cmpibe(Integer dest);
        void cmpibne(Integer dest);
        void cmpibl(Integer dest);
        void cmpible(Integer dest);
        void cmpibg(Integer dest);
        void cmpibge(Integer dest);
        void cmpobe(Integer dest);
        void cmpobne(Integer dest);
        void cmpobl(Integer dest);
        void cmpoble(Integer dest);
        void cmpobg(Integer dest);
        void cmpobge(Integer dest);
        void bbs(Integer dest);
        void bbc(Integer dest);
    private: // test condition codes
        /// @todo figure out correct signatures
        void teste(Integer dest);
        void testne(Integer dest);
        void testl(Integer dest);
        void testle(Integer dest);
        void testg(Integer dest);
        void testge(Integer dest);
    private: // call and return (note, no supervisor mode right now)
        /// @todo figure out correct signatures
        void call(Integer dest);
        void callx(Integer dest);
        void ret();
        /// @todo implement faults as exceptions
    private: // processor management
        void flushreg(); // noop right now
        void modac(RegLit mask, RegLit src, RegisterIndex dest);
    private:
        void loadRegister(Address address, int index, TreatAsOrdinal) noexcept;
        void loadRegister(Address address, int index, TreatAsInteger) noexcept;
        void loadRegister(Address address, int index, TreatAsByteInteger) noexcept;
        void loadRegister(Address address, int index, TreatAsByteOrdinal) noexcept;
        void loadRegister(Address address, int index, TreatAsShortInteger) noexcept;
        void loadRegister(Address address, int index, TreatAsShortOrdinal) noexcept;
        void storeRegister(Address address, int index, TreatAsOrdinal) noexcept;
        void storeRegister(Address address, int index, TreatAsInteger) noexcept;
        void storeRegister(Address address, int index, TreatAsByteInteger) noexcept;
        void storeRegister(Address address, int index, TreatAsByteOrdinal) noexcept;
        void storeRegister(Address address, int index, TreatAsShortInteger) noexcept;
        void storeRegister(Address address, int index, TreatAsShortOrdinal) noexcept;
        void storeLongRegister(Address address, int baseIndex) noexcept;
        void loadLongRegister(Address address, int baseIndex) noexcept;
        void storeTripleRegister(Address address, int baseIndex) noexcept;
        void loadTripleRegister(Address address, int baseIndex) noexcept;
        void storeQuadRegister(Address address, int baseIndex) noexcept;
        void loadQuadRegister(Address address, int baseIndex) noexcept;
    private:
        MemoryInterface& memoryController;
        RegisterFile globals, locals;
        Register ip; // always start at address zero
    };
}
#endif // end I960_CORE_H__

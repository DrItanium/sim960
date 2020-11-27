#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include "CoreTypes.h"
#include "TargetPlatform.h"
#include "MemoryInterface.h"
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
        using Pins = TargetBoardPinout;
    public:
        Core(MemoryInterface& mi);
        void cycle();
    private:
        Register& getRegister(int index) noexcept;
        const Register& getRegister(int index) const noexcept;
        void moveRegisterContents(int from, int to) noexcept;
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

        void mov(int dest, int src, Integer offset);
        void movl(int dest, int src, Integer offset);
        void movt(int dest, int src, Integer offset);
        void movq(int dest, int src, Integer offset);

        void lda(int dest, int src, Integer offset);
    private: // arithmetic

        void addi(int dest, int src0, int src1);
        void addo(int dest, int src0, int src1);
        void subi(int dest, int src0, int src1);
        void subo(int dest, int src0, int src1);
        void muli(int dest, int src0, int src1);
        void mulo(int dest, int src0, int src1);
        void divi(int dest, int src0, int src1);
        void divo(int dest, int src0, int src1);

        void addc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest);
        void addc(RegisterIndex src1, Literal src2, RegisterIndex dest);
        void addc(Literal src1, RegisterIndex src2, RegisterIndex dest);
        void addc(Literal src1, Literal src2, RegisterIndex dest);
        void subc(int dest, int src0, int src1);
        void emul(int dest, int src0, int src1);
        void ediv(int dest, int src0, int src1);
        void remi(int dest, int src0, int src1);
        void remo(int dest, int src0, int src1);
        void modi(int dest, int src0, int src1);

        void shlo(int dest, int src0, int src1);
        void shli(int dest, int src0, int src1);
        void shro(int dest, int src0, int src1);
        void shri(int dest, int src0, int src1);
        void shrdi(int dest, int src0, int src1);
        void rotate(int dest, int src0, int src1);

    private: // logical operations
        void logicalAnd(int dest, int src0, int src1);
        void logicalOr(int dest, int src0, int src1);
        void logicalXor(int dest, int src0, int src1);
        void logicalNor(int dest, int src0, int src1);
        void logicalNand(int dest, int src0, int src1);
        void logicalXnor(int dest, int src0, int src1);
        void logicalNot(int dest, int src0);
        void notand(int dest, int src0, int src1);
        void andnot(int dest, int src0, int src1);
        void notor(int dest, int src0, int src1);
        void ornot(int dest, int src0, int src1);
    private: // bit and bit-field operations
        void setbit(int dest, int src0, int src1);
        void clrbit(int dest, int src0, int src1);
        void notbit(int dest, int src0, int src1);
        void chkbit(int dest, int src0, int src1);
        void alterbit(int dest, int src0, int src1);
        void scanbit(int dest, int src0, int src1);
        void spanbit(int dest, int src0, int src1);
        void extract(int dest, int src0, int src1);
        void modify(int dest, int src0, int src1);
        void scanbyte(int dest, int src0, int src1);
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
        void modac(Integer dest);
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

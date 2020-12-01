#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include <tuple>
#include <variant>

#include "CoreTypes.h"
#include "TargetPlatform.h"
#include "Register.h"
#include "DependentFalse.h"
#include "ArithmeticControls.h"
namespace i960
{

    class RegFormatInstruction {
    public:
        constexpr RegFormatInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr RegLit getSrc1() const noexcept {
            // no sfr check
            if (m1) {
                return toLiteral(src1);
            } else {
                return toRegisterIndex(src1);
            }
        }
        constexpr RegLit getSrc2() const noexcept {
            // no sfr check
            if (m2) {
                return toLiteral(src2);
            } else {
                return toRegisterIndex(src2);
            }
        }
        constexpr RegisterIndex getDestination() const noexcept {
            return toRegisterIndex(srcDest);
        }
        constexpr RegLit getSrcDest() const noexcept {
            if (!m3) {
                return getDestination();
            } else {
               // can be a literal when src/dest is used as a source
               return toLiteral(srcDest);
            }
        }
        constexpr ShortOrdinal getOpcode() const noexcept {
            return ((static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0) | (static_cast<ShortOrdinal>(opcode) & 0x000F);
        }
    private:
        union {
            Ordinal _value;
            struct {
                int src1 : 5;
                int s1 : 1;
                int s2 : 1;
                int opcodeExt : 4;
                int m1 : 1;
                int m2 : 1;
                int m3 : 1;
                int src2 : 5;
                int srcDest : 5;
                int opcode : 8;
            };
        };
    };
    class COBRInstruction {
    public:
        constexpr COBRInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr RegisterIndex getSrc2() const noexcept { return toRegisterIndex(src2); }
        constexpr RegisterIndex getSrc1() const noexcept { return toRegisterIndex(src1); }
        constexpr bool getTBit() const noexcept { return t; }
        constexpr ShortInteger getDisplacement() const noexcept { return displacement; }
    private:
        union {
            Ordinal _value;
            struct {
                int s2 : 1;
                int t : 1;
                int displacement : 11;
                int m1 : 1;
                int src2 : 5;
                int src1 : 5;
                int opcode : 8;
            };
        };
    };
    class CTRLInstruction {
    public:
        constexpr CTRLInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr bool getTBit() const noexcept { return t; }
        constexpr Integer getDisplacement() const noexcept { return displacement; }
    private:
        union {
            Ordinal _value;
            struct {
                int unused : 1;
                int t : 1;
                int displacement : 22;
                int opcode : 8;
            };
        };
    };
    class Core;
    class MEMFormatInstruction {
    public:
        constexpr explicit MEMFormatInstruction(Ordinal lowerHalf) noexcept : lower(lowerHalf) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return static_cast<ShortOrdinal>(opcode) << 4; }
        Ordinal computeAddress(Core& referenceCore) const noexcept;
        constexpr RegisterIndex getSrcDest() const noexcept { return static_cast<RegisterIndex>(srcDest); }
    private:
        constexpr bool isMEMAFormat() const noexcept { return modeMajor & 1 == 0; }
        constexpr bool isMEMBFormat() const noexcept { return modeMajor & 1 != 0; }
        Ordinal computeAddress_MEMA(Core& referenceCore) const noexcept;
        Ordinal computeAddress_MEMB(Core& referenceCore) const noexcept;
        Ordinal computeScale(Core& referenceCore) const noexcept;
    private:
        union {
            Ordinal lower;
            struct {
                // generic view
                unsigned int differentiationBlock : 12;
                unsigned int modeMajor : 2;
                unsigned int abase : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
            };
            struct {
                unsigned int offset : 12;
                unsigned int mode : 2;
                unsigned int abase : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
            } mema;
            struct {
                unsigned int index : 5;
                unsigned int unused0 : 2;
                unsigned int scale : 3;
                unsigned int mode : 4;
                unsigned int abase : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
            } memb;
        };
    };
    using RegisterFile = std::array<Register, 16>;
    class Core {
    private:
        using DecodedInstruction = std::variant<RegFormatInstruction,
                MEMFormatInstruction,
                COBRInstruction,
                CTRLInstruction>;
        static DecodedInstruction decode(Ordinal value) noexcept;
    public:
        void begin();
        void cycle();
        Register& getRegister(int index) noexcept;
        inline Register& getRegister(RegisterIndex index) noexcept { return getRegister(toInteger(index)); }
        const Register& getRegister(int index) const noexcept;
        inline const Register& getRegister(RegisterIndex index) const noexcept { return getRegister(toInteger(index)); }
        const Register& getIP() const noexcept { return ip; }
        /**
         * @brief Retrieve the word at the ip address
         * @param advance
         * @return
         */
        Ordinal getWordAtIP(bool advance = false) noexcept;
    private:
        // classic risc pipeline stages
        /// @todo flesh out
        Ordinal fetchInstruction();
        DecodedInstruction decodeInstruction(Ordinal value);
        void executeInstruction(const DecodedInstruction& inst);
        void memoryAccess();
        void writeback();
    private: // execution routines
        void execute(const RegFormatInstruction& inst);
        void execute(const COBRInstruction& inst);
        void execute(const CTRLInstruction& inst);
        void execute(const MEMFormatInstruction& inst);
    private: // common internal functions
        void saveLocals() noexcept;
        void restoreLocals() noexcept;
        bool getCarryFlag() const noexcept;
        void setCarryFlag(bool value) noexcept;
    private: // memory controller interface routines for abstraction purposes
        Ordinal loadOrdinal(Address address) noexcept { return theBoard.loadValue(address, TreatAsOrdinal{}); }
        void storeOrdinal (Address address, Ordinal value) noexcept { theBoard.storeValue(address, value, TreatAsOrdinal{}); }
        Integer loadInteger(Address address) noexcept { return theBoard.loadValue(address, TreatAsInteger{}); }
        void storeInteger (Address address, Integer value) noexcept { theBoard.storeValue(address, value, TreatAsInteger{}); }

        ByteOrdinal loadByteOrdinal(Address address) noexcept { return theBoard.loadValue(address, TreatAsByteOrdinal{}); }
        void storeByteOrdinal (Address address, ByteOrdinal value) noexcept { theBoard.storeValue(address, value, TreatAsByteOrdinal{}); }
        ByteInteger loadByteInteger(Address address) noexcept { return theBoard.loadValue(address, TreatAsByteInteger{}); }
        void storeByteInteger (Address address, ByteInteger value) noexcept { theBoard.storeValue(address, value, TreatAsByteInteger{}); }

        ShortOrdinal loadShortOrdinal(Address address) noexcept { return theBoard.loadValue(address, TreatAsShortOrdinal{}); }
        void storeShortOrdinal (Address address, ShortOrdinal value) noexcept { theBoard.storeValue(address, value, TreatAsShortOrdinal{}); }
        ShortInteger loadShortInteger(Address address) noexcept { return theBoard.loadValue(address, TreatAsShortInteger{}); }
        void storeShortInteger (Address address, ShortInteger value) noexcept { theBoard.storeValue(address, value, TreatAsShortInteger{}); }
    private: // data movement operations
        // mem reg {
        void ld(Ordinal mem, RegisterIndex dest);
        void ldob(Ordinal mem, RegisterIndex dest);
        void ldos(Ordinal mem, RegisterIndex dest);
        void ldib(Ordinal mem, RegisterIndex dest);
        void ldis(Ordinal mem, RegisterIndex dest);
        void ldl(Ordinal mem, RegisterIndex dest);
        void ldt(Ordinal mem, RegisterIndex dest);
        void ldq(Ordinal mem, RegisterIndex dest);
        void lda(Ordinal mem, RegisterIndex dest); // efa is another accepted value

        void st(RegisterIndex src, Ordinal mem);
        void stob(RegisterIndex src, Ordinal mem);
        void stos(RegisterIndex src, Ordinal mem);
        void stib(RegisterIndex src, Ordinal mem);
        void stis(RegisterIndex src, Ordinal mem);
        void stl(RegisterIndex src, Ordinal mem);
        void stt(RegisterIndex src, Ordinal mem);
        void stq(RegisterIndex src, Ordinal mem);

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
        void b(Displacement22 targ);
        void bx(Ordinal targ);
        void bal(Displacement22 targ);
        void balx(Ordinal targ, RegisterIndex dest); // mem, reg

        /// @todo figure out correct signatures
        void be(Displacement22 dest);
        void bne(Displacement22 dest);
        void bl(Displacement22 dest);
        void ble(Displacement22 dest);
        void bg(Displacement22 dest);
        void bge(Displacement22 dest);
    private: // compare and branch
        /// @todo figure out correct signatures
        void cmpibe(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobe(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibne(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobne(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibl(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobl(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpible(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpoble(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibg(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobg(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibge(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobge(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void bbs(RegLit bitpos, RegisterIndex src, ShortInteger targ);
        void bbc(RegLit bitpos, RegisterIndex src, ShortInteger targ);
    private: // test condition codes
        void teste(RegisterIndex dest);
        void testne(RegisterIndex dest);
        void testl(RegisterIndex dest);
        void testle(RegisterIndex dest);
        void testg(RegisterIndex dest);
        void testge(RegisterIndex dest);
    private: // call and return (note, no supervisor mode right now)
        /// @todo figure out correct signatures
        void call(Displacement22 targ);
        void callx(Ordinal targ); // mem
        void ret();
        /// @todo implement faults as exceptions
    private: // processor management
        void flushreg(); // noop right now
        void modac(const RegFormatInstruction& inst);
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
        TargetBoard theBoard; // default constructible
        RegisterFile globals, locals;
        Register ip; // always start at address zero
        ArithmeticControls ac;
        bool _unalignedFaultEnabled = false;
    };

}
#endif // end I960_CORE_H__

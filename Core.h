#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include <tuple>
#include <variant>
#include <algorithm>

#include "CoreTypes.h"
#include "Register.h"
#include "DependentFalse.h"
#include "ArithmeticControls.h"
#include "ProcessControls.h"
#include "Disassembly.h"
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
            return ((static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0) | (static_cast<ShortOrdinal>(opcodeExt) & 0x000F);
        }
        std::string decodeName() const noexcept;
        constexpr auto lowerHalf() const noexcept { return _value; }
    private:
        union {
            Ordinal _value;
            struct {
                unsigned int src1 : 5;
                unsigned int s1 : 1;
                unsigned int s2 : 1;
                unsigned int opcodeExt : 4;
                unsigned int m1 : 1;
                unsigned int m2 : 1;
                unsigned int m3 : 1;
                unsigned int src2 : 5;
                unsigned int srcDest : 5;
                unsigned int opcode : 8;
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
        std::string decodeName() const noexcept;
        constexpr auto lowerHalf() const noexcept { return _value; }
    private:
        union {
            Ordinal _value;
            struct {
                unsigned int s2 : 1;
                unsigned int t : 1;
                int displacement : 11;
                unsigned int m1 : 1;
                unsigned int src2 : 5;
                unsigned int src1 : 5;
                unsigned int opcode : 8;
            };
        };
    };
    class CTRLInstruction {
    public:
        constexpr CTRLInstruction(Ordinal value) noexcept : _value(value) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr bool getTBit() const noexcept { return _value & 0b10; }
        constexpr Integer getDisplacement() const noexcept { return displacement & 0x00FF'FFFC; }
        std::string decodeName() const noexcept;
        constexpr auto lowerHalf() const noexcept { return _value; }
    private:
        union {
            Ordinal _value;
            struct {
                int displacement : 24;
                unsigned int opcode : 8;
            };
        };
    };
    class MEMFormatInstruction {
    public:
        constexpr explicit MEMFormatInstruction(Ordinal lowerHalf, Ordinal upperHalf) noexcept : lower(lowerHalf), next(upperHalf) { }
        constexpr ShortOrdinal getOpcode() const noexcept { return static_cast<ShortOrdinal>(opcode) << 4; }
        template<typename Core>
        Ordinal computeAddress(Core& referenceCore) const noexcept {
            if (isMEMAFormat()) {
                return computeAddress_MEMA(referenceCore);
            } else {
                return computeAddress_MEMB(referenceCore);
            }
        }
        constexpr RegisterIndex getSrcDest() const noexcept { return static_cast<RegisterIndex>(srcDest); }
        std::string decodeName() const noexcept;
        constexpr auto upperHalf() const noexcept { return next; }
        constexpr auto lowerHalf() const noexcept { return lower; }
    private:
        constexpr bool isMEMAFormat() const noexcept { return modeMajor & 1 == 0; }
        constexpr bool isMEMBFormat() const noexcept { return modeMajor & 1 != 0; }
        template<typename Core>
        Ordinal computeAddress_MEMA(Core& referenceCore) const noexcept {
            // the lsb of mema.mode will always be 0 to get to this point
            if (mema.mode == 0b00) {
                // absolute offset
                return static_cast<Ordinal>(mema.offset);
            } else {
                // register indirect with offset
                return static_cast<Ordinal>(mema.offset) + referenceCore.getRegister(toRegisterIndex(abase)).getOrdinal();
            }

        }
        template<typename Core>
        Ordinal computeAddress_MEMB(Core& referenceCore) const noexcept {
            // bit 12 of the instruction has to be 1 to get into this code, eight possible combos
            switch (memb.mode) {
                case 0b0100: // register indirect
                    return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal();
                case 0b0101: // ip with displacement
                    referenceCore.nextInstruction();
                    return static_cast<Ordinal>(referenceCore.getIP().getInteger() + optionalDisplacement + 8);
                case 0b0110: // reserved
                    return -1;
                case 0b0111: // register indirect with index
                    return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal() +
                           referenceCore.getRegister(toRegisterIndex(memb.index)).getOrdinal() *
                           computeScale(referenceCore);
                case 0b1100: // absolute displacement
                    referenceCore.nextInstruction();
                    return static_cast<Ordinal>(optionalDisplacement);
                case 0b1101: // register indirect with displacement
                    referenceCore.nextInstruction();
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() + optionalDisplacement);
                case 0b1110: // index with displacement
                    referenceCore.nextInstruction();
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) + optionalDisplacement);
                case 0b1111: // register indirect with index and displacement
                    referenceCore.nextInstruction();
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                                referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) + optionalDisplacement);
                default:
                    return -1;
            }
        }
        template<typename Core>
        Ordinal computeScale(Core& referenceCore) const noexcept {

            switch (memb.scale) {
                case 0b000: return 1;
                case 0b001: return 2;
                case 0b010: return 4;
                case 0b011: return 8;
                case 0b100: return 16;
                default:
                    referenceCore.raiseFault(); // invalid opcode
                    return 1;
            }
        }
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
        union {
            Ordinal next;
            Integer optionalDisplacement;
        };
    };
    class Core {
    private:
        static constexpr Ordinal computeSingleBitShiftMask(Ordinal value) noexcept {
            return 1 << (value & 0b11111);
        }

        static constexpr Ordinal rotateOperation(Ordinal src, Ordinal length) noexcept {
            return (src << length) | (src >> ((-length) & 31u));
        }
    private:
        using DecodedInstruction = std::variant<RegFormatInstruction,
                MEMFormatInstruction,
                COBRInstruction,
                CTRLInstruction>;
        static DecodedInstruction decode(Ordinal lower, Ordinal upper) noexcept;
    public:
        using RegisterFile = std::array<Register, 16>;
    public:
        /**
         * @brief Construct a core object
         * @param ibrBase The address of the initialization boot record
         * @param salign The stack alignment value (defaults to 1)
         */
        explicit Core(Ordinal ibrBase, unsigned int salign = 1) : _salign(salign), _ibrBase(ibrBase) { }
        constexpr Ordinal computeAlignmentBoundaryConstant() const noexcept {
            return (_salign * 16) - 1;
        }
        /**
         * @brief Perform the power on self test, get the image information, and then return
         */
        void post();
        void cycle();
        void cycle(Ordinal lower, Ordinal upper = 0);
        Register& getRegister(RegisterIndex index) noexcept;
        const Register& getRegister(RegisterIndex index) const noexcept;
        const Register& getIP() const noexcept { return ip; }
        /**
         * @brief Retrieve the word at the ip address
         * @param advance
         * @return
         */
        Ordinal getWordAtIP(bool advance = false) noexcept;
    private: // memory controller interface routines for abstraction purposes, must be implemented in the .ino file
        Ordinal loadOrdinal(Address address) noexcept;
        void storeOrdinal (Address address, Ordinal value) noexcept;
        Integer loadInteger(Address address) noexcept;
        void storeInteger (Address address, Integer value) noexcept;

        ByteOrdinal loadByteOrdinal(Address address) noexcept;
        void storeByteOrdinal (Address address, ByteOrdinal value) noexcept;
        ByteInteger loadByteInteger(Address address) noexcept;
        void storeByteInteger (Address address, ByteInteger value);

        ShortOrdinal loadShortOrdinal(Address address) noexcept;
        void storeShortOrdinal (Address address, ShortOrdinal value) noexcept;
        ShortInteger loadShortInteger(Address address) noexcept;
        void storeShortInteger (Address address, ShortInteger value) noexcept;
    private:
        void executeInstruction(const DecodedInstruction& inst);
    public: // fault related
        void raiseFault();
    private: // execution routines
        void execute(const RegFormatInstruction& inst) noexcept;
        void execute(const MEMFormatInstruction &inst) noexcept;
        void execute(const COBRInstruction &inst) noexcept;
        void execute(const CTRLInstruction &inst) noexcept;

    private: // common internal functions
        void saveLocals() noexcept;
        void restoreLocals() noexcept;
        bool getCarryFlag() const noexcept;
        void setCarryFlag(bool value) noexcept;
    private: // data movement operations
        void lda(Ordinal mem, RegisterIndex dest);
        void ld(Ordinal address, RegisterIndex dest);
        void ldob(Ordinal mem, RegisterIndex dest);
        void ldos(Ordinal mem, RegisterIndex dest);
        void ldib(Ordinal mem, RegisterIndex dest);
        void ldis(Ordinal mem, RegisterIndex dest);
        void st(RegisterIndex src, Ordinal dest);
        void stob(RegisterIndex src, Ordinal dest);
        void stib(RegisterIndex src, Ordinal dest);
        void stis(RegisterIndex src, Ordinal dest);
        void stos(RegisterIndex src, Ordinal dest);
        void stl(RegisterIndex src, Ordinal address);
        void stt(RegisterIndex src, Ordinal address);
        void stq(RegisterIndex src, Ordinal address);
        void ldl(Ordinal mem, RegisterIndex dest);
        void ldt(Ordinal mem, RegisterIndex dest);
        void ldq(Ordinal mem, RegisterIndex dest);
        void mov(RegLit src, RegisterIndex dest);
        void movl(RegLit src, RegisterIndex dest);
        void movt(RegLit src, RegisterIndex dest);
        void movq(RegLit src, RegisterIndex dest);
    private: // internals
        Ordinal extractValue(RegLit value, TreatAsOrdinal) const noexcept;
        Integer extractValue(RegLit value, TreatAsInteger) const noexcept;
        RegLit nextValue(RegLit value) const noexcept;
    private: // arithmetic
        void addc(RegLit src1, RegLit src2, RegisterIndex dest);
        void addi(RegLit src1, RegLit src2, RegisterIndex dest);
        void addo(RegLit src1, RegLit src2, RegisterIndex dest);
        void subi(RegLit src1, RegLit src2, RegisterIndex dest);
        void subo(RegLit src1, RegLit src2, RegisterIndex dest);
        void subc(RegLit src1, RegLit src2, RegisterIndex dest);
        void muli(RegLit src1, RegLit src2, RegisterIndex dest);
        void mulo(RegLit src1, RegLit src2, RegisterIndex dest);
        void divi(RegLit src1, RegLit src2, RegisterIndex dest);
        void divo(RegLit src1, RegLit src2, RegisterIndex dest);
        void ediv(RegLit src1, RegLit src2, RegisterIndex dest);
        void emul(RegLit src1, RegLit src2, RegisterIndex dest);
        void remi(RegLit src1, RegLit src2, RegisterIndex dest);
        void remo(RegLit src1, RegLit src2, RegisterIndex dest);
        void modi(RegLit src1, RegLit src2, RegisterIndex dest);
        void shlo(RegLit len, RegLit src, RegisterIndex dest);
        void shro(RegLit len, RegLit src, RegisterIndex dest);
        void shli(RegLit len, RegLit src, RegisterIndex dest);
        void shri(RegLit src1, RegLit src2, RegisterIndex dest);
        void shrdi(RegLit src1, RegLit src2, RegisterIndex dest);
        void rotate(RegLit src1, RegLit src2, RegisterIndex dest);

    private: // logical operations
        void logicalAnd(RegLit src1, RegLit src2, RegisterIndex dest);
        void andnot(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalNand(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalNor(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalNot(RegLit src, RegisterIndex dest);
        void notand(RegLit src1, RegLit src2, RegisterIndex dest);
        void notor(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalOr(RegLit src1, RegLit src2, RegisterIndex dest);
        void ornot(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalXor(RegLit src1, RegLit src2, RegisterIndex dest);
        void logicalXnor(RegLit src1, RegLit src2, RegisterIndex dest);
    private: // bit and bit-field operations
        void setbit(RegLit src1, RegLit src2, RegisterIndex dest);
        void clrbit(RegLit src1, RegLit src2, RegisterIndex dest);
        void notbit(RegLit src1, RegLit src2, RegisterIndex dest);
        void alterbit(RegLit src1, RegLit src2, RegisterIndex dest);
        void chkbit(RegLit src1, RegLit src2);
        void spanbit(RegLit src1, RegisterIndex dest);
        void scanbit(RegLit src, RegisterIndex dest);
        void extract(RegLit src1, RegLit src2, RegisterIndex dest);
        void modify(RegLit mask, RegLit src, RegisterIndex srcDest);
        void scanbyte(RegLit src1, RegLit src2);
    private: // compare and increment or decrement
        template<typename Tag>
        void
        compareBase(RegLit src1, RegLit src2) {
            auto s1 = extractValue(src1, Tag{});
            auto s2 = extractValue(src2, Tag{});
            if (s1 < s2) {
                ac.setConditionCode(0b100);
            } else if (s1 == s2) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b001);
            }
        }
        template<typename Tag>
        void
        conditionalCompareBase(RegLit src1, RegLit src2) {
            // don't care what the least significant two bits are of the cond code so just mask them out
            if ((ac.getConditionCode() & 0b100) == 0) {
                auto s1 = extractValue(src1, Tag{});
                auto s2 = extractValue(src2, Tag{});
                ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
            }
        }
        template<typename Tag>
        void
        compareAndIncrementBase(RegLit src1, RegLit src2, RegisterIndex dest) {
            compareBase<Tag>(src1, src2);
            getRegister(dest).set(extractValue(src2, Tag{}) + 1, Tag{});
        }
        template<typename Tag>
        void
        compareAndDecrementBase(RegLit src1, RegLit src2, RegisterIndex dest) {
            compareBase<Tag>(src1, src2);
            getRegister(dest).set(extractValue(src2, Tag{}) - 1, Tag{});
        }
        inline void cmpo(RegLit src1, RegLit src2) { compareBase<TreatAsOrdinal>(src1, src2); }
        inline void cmpi(RegLit src1, RegLit src2) { compareBase<TreatAsInteger>(src1, src2); }
        void concmpo(RegLit src1, RegLit src2);
        void concmpi(RegLit src1, RegLit src2);
        void cmpinco(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpinci(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest);
        void cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest);
    private: // branching
        void b(Displacement22 targ);
        void bal(Displacement22 targ);
        void bx(Ordinal targ);
        void balx(Ordinal targ, RegisterIndex dest);
    private: // condensed logic forms
        /// @todo condense this operation set down to a single function since the mask is embedded in the opcode itself :)
        template<ConditionCodeKind cck>
        void
        conditionalBranch(Displacement22 dest) {
            if (ac.conditionIs<cck>()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        template<ConditionCodeKind cck>
        void
        conditionalFault() {
            if (ac.conditionIs<cck>()) {
                raiseFault();
            }
        }
        template<ConditionCodeKind cck>
        void
        compareAndBranch(RegLit src1, RegisterIndex src2, ShortInteger targ, TreatAsInteger) {
            cmpi(src1, src2);
            conditionalBranch<cck>(Displacement22{targ});
        }
        template<ConditionCodeKind cck>
        void
        compareAndBranch(RegLit src1, RegisterIndex src2, ShortInteger targ, TreatAsOrdinal) {
            cmpo(src1, src2);
            conditionalBranch<cck>(Displacement22{targ});
        }
    private: // compare and branch
        void bbc(RegLit bitpos, RegisterIndex src, ShortInteger targ);
        void bbs(RegLit bitpos, RegisterIndex src, ShortInteger targ);
    private: // test condition codes
        template<ConditionCodeKind cck>
        void
        testBase(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIs<cck>() ? 1 : 0);
        }
    private:
        Ordinal getStackPointerAddress() const noexcept;
        void setRIP(const Register& ip) noexcept;
        Ordinal getFramePointerAddress() const noexcept;
        void setPFP(Ordinal value) noexcept;
        void setFramePointer(Ordinal value) noexcept;
        void setStackPointer(Ordinal value) noexcept;
        void allocateNewLocalRegisterSet();
    private: // call and return
        void call(Displacement22 targ);
        void callx(Ordinal targ);
        void calls(RegLit targ);
        void ret();
    private: // processor management
        void flushreg();
        void modtc(const RegFormatInstruction& inst);
        void modpc(const RegFormatInstruction& inst);
        void modac(const RegFormatInstruction& inst);
        void syncf();
    private: // marking
        void fmark();
        void mark();
    private: // Numerics Architecture addons
        void dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest);
        void dmovt(RegisterIndex src1, RegisterIndex dest);
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
    public:
        void nextInstruction();
    private:
        void badInstruction(DecodedInstruction inst);
    private:
        RegisterFile globals, locals;
        Register ip; // always start at address zero
        ArithmeticControls ac;
        ProcessControls pc;
        bool _unalignedFaultEnabled = false;
        unsigned int _salign = 1;
        Ordinal _ibrBase = 0;
    };

}
#endif // end I960_CORE_H__

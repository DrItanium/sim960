#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include <tuple>
#include <variant>
#include <algorithm>
#include <memory>
#include <iostream>

#include "CoreTypes.h"
#include "Register.h"
#include "DependentFalse.h"
#include "ArithmeticControls.h"
#include "ProcessControls.h"
#include "Disassembly.h"
#include "TraceControls.h"
#include "PreviousFramePointer.h"
#include "BusInterfaceUnit.h"
#include "InstructionFormats.h"
namespace i960
{

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
         * @param ibrBase The address of the initialization boot record (defaults to 0xFEFF'FF30, same as the i960 Hx)
         * @param salign The stack alignment value (defaults to 1)
         */
        explicit Core(BusInterfaceUnit& biu, Ordinal ibrBase = 0xFEFF'FF30, Ordinal salign = 1) : biu_(biu), salign_(salign), ibrBase_(ibrBase) { }
        constexpr Ordinal computeAlignmentBoundaryConstant() const noexcept {
            return (salign_ * 16) - 1;
        }
        /**
         * @brief Perform the power on self test, get the image information, and then return
         */
        void post();
        void cycle();
        void cycle(Ordinal lower, Ordinal upper = 0);
        [[nodiscard]] Register& getRegister(RegisterIndex index) noexcept;
        [[nodiscard]] const Register& getRegister(RegisterIndex index) const noexcept;
        [[nodiscard]] const Register& getIP() const noexcept { return ip; }
        [[noreturn]] void busTestFailed() noexcept;
        /**
         * @brief Retrieve the word at the ip address
         * @param advance
         * @return
         */
        Ordinal getWordAtIP(bool advance = false) noexcept;
    private: // memory controller interface routines for abstraction purposes, must be implemented in the .ino file
        InterfaceUnit& getInterfaceUnit(Address address) noexcept;
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
        void checkPendingInterrupts() noexcept;
        void freeCurrentRegisterSet() noexcept;
        void saveRegisterSet() noexcept;
        void restoreRegisterSet() noexcept;
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
        void cmpo(RegLit src1, RegLit src2);
        void cmpi(RegLit src1, RegLit src2);
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
                auto dv = dest.getValue();
                auto oldLoc = ip.getInteger();
                auto newLoc = (oldLoc + dv) & (~0b11);
                ip.setInteger(newLoc);
                doNotAdvanceIp();
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
        [[nodiscard]] PreviousFramePointer getPFP() noexcept;
        [[nodiscard]] Register& getStackPointer() noexcept;
        [[nodiscard]] Register& getFramePointer() noexcept;
        [[nodiscard]] Register& getReturnInstructionPointer() noexcept;

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
    private: // i960 Hx,Jx extended instructions (partial set)
        template<ConditionCodeKind cck>
        void selectGeneral(RegLit src1, RegLit src2, RegisterIndex dest) noexcept {
            if ((ac.conditionIs<cck>()) || (static_cast<ByteOrdinal>(cck) == ac.getConditionCode())) {
                getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}));
            } else {
                getRegister(dest).setOrdinal(extractValue(src1, TreatAsOrdinal{}));
            }
        }
        inline void selno(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::Unordered>(src1, src2, dest); }
        inline void selg(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::GreaterThan>(src1, src2, dest); }
        inline void sele(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::EqualTo>(src1, src2, dest); }
        inline void selge(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::GreaterThanOrEqualTo>(src1, src2, dest); }
        inline void sell(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::LessThan>(src1, src2, dest); }
        inline void selne(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::NotEqual>(src1, src2, dest); }
        inline void selle(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::LessThanOrEqual>(src1, src2, dest); }
        inline void selo(RegLit src1, RegLit src2, RegisterIndex dest) noexcept { selectGeneral<ConditionCodeKind::Ordered>(src1, src2, dest); }

    public:
        /**
         * @brief Set ip to the nearest word aligned address
         * @param address Where ip should be, it is word aligned internally
         */
        void setIP(Address address) noexcept { ip.setOrdinal(address & (~0b11)); }
        void setSP(Address address) noexcept { getStackPointer().setOrdinal(address); }
        void setFP(Address address) noexcept { getFramePointer().setOrdinal(address); }
        void setFrameStart(Address address) noexcept { setFP(address); setSP(address + 64); }
    private:
        void badInstruction(DecodedInstruction inst);
        [[nodiscard]] Ordinal getSystemProcedureEntry(Ordinal targ) noexcept;
        [[nodiscard]] bool registerSetAvailable() const noexcept;
        [[nodiscard]] bool registerSetNotAllocated(Ordinal address) const noexcept;
        [[nodiscard]] Ordinal getSupervisorStackPointer() noexcept;
        [[nodiscard]] Ordinal getSystemProcedureTableBase() noexcept;
    private:
        Ordinal bootChecksum() noexcept;
    private:
        void processPRCB() noexcept;
    private:
        void cmpobg(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibg(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobge(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibge(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobe(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibe(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobne(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibne(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpobl(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibl(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpoble(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpible(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibno(RegLit src1, RegisterIndex src2, ShortInteger targ);
        void cmpibo(RegLit src1, RegisterIndex src2, ShortInteger targ);
    public:
        /**
         * @brief Tell the ip incrementer that the instruction is double wide (increment by 8 instead)
         */
        void instructionIsDoubleWide() noexcept;
        /**
         * @brief Tell the ip incrementer that the instruction prevents ip from being implicitly incremented by four
         */
        void doNotAdvanceIp() noexcept;
    private:
        BusInterfaceUnit& biu_;
        RegisterFile globals, locals;
        Register ip; // always start at address zero
        ArithmeticControls ac;
        ProcessControls pc;
        TraceControls tc;
        bool _unalignedFaultEnabled = true;
        unsigned int salign_ = 1;
        Ordinal ibrBase_ = 0;
        Ordinal prcbBase_ = 0;
        Ordinal ctrlTableBase_ = 0;
        Ordinal faultTableBase_ = 0;
        Ordinal interruptTableBase_ = 0;
        Ordinal nmiVector_ = 0;
        // no on die ram yet, at some point I may consider doing this, I have no idea how much space different peripherals will take
        Ordinal ipIncrement_ = 4;

    };

}
#endif // end I960_CORE_H__

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
    class MEMFormatInstruction {
    public:
        constexpr explicit MEMFormatInstruction(Ordinal lowerHalf) noexcept : lower(lowerHalf) { }
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
            union {
                Ordinal next;
                Integer optionalDisplacement;
            } disp;
            disp.next = 0;
            switch (memb.mode) {
                case 0b0100: // register indirect
                    return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal();
                case 0b0101: // ip with displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getIP().getInteger() + disp.optionalDisplacement + 8);
                case 0b0110: // reserved
                    return -1;
                case 0b0111: // register indirect with index
                    return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal() +
                           referenceCore.getRegister(toRegisterIndex(memb.index)).getOrdinal() *
                           computeScale(referenceCore);
                case 0b1100: // absolute displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(disp.optionalDisplacement);
                case 0b1101: // register indirect with displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                                disp.optionalDisplacement);
                case 0b1110: // index with displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) +
                                                disp.optionalDisplacement);
                case 0b1111: // register indirect with index and displacement
                    disp.next = referenceCore.getWordAtIP(true);
                    return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                                referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) +
                                                disp.optionalDisplacement);
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
                    /// @todo raise an invalid opcode fault here using referenceCore
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
        static DecodedInstruction decode(Ordinal value) noexcept {
            if (auto opcode = static_cast<ByteOrdinal>(value >> 24); opcode < 0x20) {
                return CTRLInstruction(value);
            } else if (opcode >= 0x20 && opcode < 0x58) {
                return COBRInstruction(value);
            } else if (opcode >= 0x58 && opcode < 0x80) {
                return RegFormatInstruction(value);
            } else {
                return MEMFormatInstruction(value);
            }
        }
    public:
        using RegisterFile = std::array<Register, 16>;
    public:
        explicit Core(unsigned int salign = 1) : _salign(salign) { }
        constexpr Ordinal computeAlignmentBoundaryConstant() const noexcept {
            return (_salign * 16) - 1;
        }
        void cycle() {
            executeInstruction(decodeInstruction(fetchInstruction()));
        }
        Register& getRegister(int index) noexcept {
            if (auto offset = index & 0b1111, maskedValue = index & 0b10000; maskedValue != 0) {
                return locals[offset];
            } else {
                return globals[offset];
            }
        }
        inline Register& getRegister(RegisterIndex index) noexcept { return getRegister(toInteger(index)); }
        const Register& getRegister(int index) const noexcept {
            if (auto offset = index & 0b1111, maskedValue = index & 0b10000; maskedValue != 0) {
                return locals[offset];
            } else {
                return globals[offset];
            }
        }
        inline const Register& getRegister(RegisterIndex index) const noexcept { return getRegister(toInteger(index)); }
        const Register& getIP() const noexcept { return ip; }
        /**
         * @brief Retrieve the word at the ip address
         * @param advance
         * @return
         */
        Ordinal
        getWordAtIP(bool advance = false) noexcept {
            auto ipLoc = ip.getOrdinal();
            if (advance) {
                ip.setOrdinal(ipLoc + 4);
            }
            return loadOrdinal(ipLoc);
        }
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
        // classic risc pipeline stages
        DecodedInstruction
        decodeInstruction(Ordinal currentInstruction) {
            return decode(currentInstruction);
        }
        void
        executeInstruction(const DecodedInstruction& inst) {
            std::visit([this](auto&& theInst) { execute(theInst); }, inst);
        }
        Ordinal
        fetchInstruction() {
            return getWordAtIP(true);
        }
    private: // fault related
        void
        raiseFault() {
           /// @todo flesh this out
        }
    private: // execution routines
        void
        execute(const RegFormatInstruction& inst) noexcept {
            switch(inst.getOpcode()) {
                case 0x580: notbit(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x581: logicalAnd(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x582: andnot(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x583: setbit(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x584: notand(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x586: logicalXor(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x587: logicalOr(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x588: logicalNor(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x589: logicalXnor(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x58A: logicalNot(inst.getSrc1(), inst.getDestination()); break;
                case 0x58B: ornot(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x58C: clrbit(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x58D: notor(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x58E: logicalNand(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x58F: alterbit(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x590: addo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x591: addi(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x592: subo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x593: subi(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x594: cmpob(inst.getSrc1(), inst.getSrc2()); break;
                    //case 0x595: cmpib(inst.getSrc1(), inst.getSrc2()); break;
                    //case 0x596: cmpos(inst.getSrc1(), inst.getSrc2()); break;
                    //case 0x597: cmpis(inst.getSrc1(), inst.getSrc2()); break;
                case 0x598: shro(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x59A: shrdi(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x59B: shri(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x59C: shlo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x59D: rotate(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x59E: shli(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x5A0: cmpo(inst.getSrc1(), inst.getSrc2()); break;
                case 0x5A1: cmpi(inst.getSrc1(), inst.getSrc2()); break;
                case 0x5A2: concmpo(inst.getSrc1(), inst.getSrc2()); break;
                case 0x5A3: concmpi(inst.getSrc1(), inst.getSrc2()); break;
                case 0x5A4: cmpinco(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x5A5: cmpinci(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x5A6: cmpdeco(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x5A7: cmpdeci(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x5AC: scanbyte(inst.getSrc1(), inst.getSrc2()); break;
                //case 0x5AD: bswap(inst.getSrc1(), inst.getSrc2()); break;
                case 0x5AE: chkbit(inst.getSrc1(), inst.getSrc2()); break;
                case 0x5B0: addc(inst.getSrc1(), inst.getSrc2(),inst.getDestination()); break;
                case 0x5B2: subc(inst.getSrc1(), inst.getSrc2(),inst.getDestination()); break;
                    // case 0x5B4: intdis(); break;
                    // case 0x5B5: inten(); break;
                case 0x5CC: mov(inst.getSrc1(), inst.getDestination()); break;
                case 0x5D8: eshro(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x5DC: movl(inst.getSrc1(), inst.getDestination()); break;
                case 0x5EC: movt(inst.getSrc1(), inst.getDestination()); break;
                case 0x5FC: movq(inst.getSrc1(), inst.getDestination()); break;
                //case 0x610: atmod(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x612: atadd(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x640: spanbit(inst.getSrc1(), inst.getDestination()); break;
                case 0x641: scanbit(inst.getSrc1(), inst.getDestination()); break;
                case 0x642: daddc(std::get<RegisterIndex>(inst.getSrc1()), std::get<RegisterIndex>(inst.getSrc2()), inst.getDestination()); break;
                case 0x643: dsubc(std::get<RegisterIndex>(inst.getSrc1()), std::get<RegisterIndex>(inst.getSrc2()), inst.getDestination()); break;
                case 0x644: dmovt(std::get<RegisterIndex>(inst.getSrc1()), inst.getDestination()); break;
                case 0x645: modac(inst); break;
                case 0x650: modify(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x651: extract(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x654: modtc(inst); break;
                case 0x655: modpc(inst); break;
                    //case 0x658: intctl(inst.getSrc1(), inst.getDestination()); break;
                    //case 0x659: sysctl(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x65B: icctl(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x65C: dcctl(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x660: calls(inst.getSrc1()); break;
                case 0x66B: mark(); break;
                case 0x66C: fmark(); break;
                case 0x66D: flushreg(); break;
                case 0x66F: syncf(); break;
                case 0x670: emul(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x671: ediv(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x701: mulo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x708: remo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x70B: divo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x741: muli(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x748: remi(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x749: modi(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x74B: divi(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x780: addono(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x781: addino(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x782: subono(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x783: subino(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x784: selno(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x790: addog(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x791: addig(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x792: subog(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x793: subig(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x794: selg(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7A0: addoe(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7A1: addie(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7A2: suboe(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7A3: subie(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7A4: sele(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7B0: addoge(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7B1: addige(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7B2: suboge(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7B3: subige(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7B4: selge(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7C0: addol(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7C1: addil(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7C2: subol(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7C3: subil(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7C4: sell(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7D0: addone(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7D1: addine(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7D2: subone(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7D3: subine(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7D4: selne(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7E0: addole(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7E1: addile(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7E2: subole(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7E3: subile(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7E4: selle(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7F0: addoo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7F1: addio(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7F2: suboo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7F3: subio(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                    //case 0x7F4: selo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                default:
                    raiseFault();
                    /// @todo raise an error at this point
                    break;
            }
        }
        void
        execute(const MEMFormatInstruction &inst) noexcept {
            auto address = inst.computeAddress(*this);
            switch (inst.getOpcode()) {
                case 0x800: ldob(address, inst.getSrcDest()); break;
                case 0x820: stob(inst.getSrcDest(), address); break;
                case 0x840: bx(address); break;
                case 0x850: balx(address, inst.getSrcDest()); break;
                case 0x860: callx(address); break;
                case 0x880: ldos(address, inst.getSrcDest()); break;
                case 0x8A0: stos(inst.getSrcDest(), address); break;
                case 0x8C0: lda(address, inst.getSrcDest()); break;
                case 0x900: ld(address, inst.getSrcDest()); break;
                case 0x920: st(inst.getSrcDest(), address); break;
                case 0x980: ldl(address, inst.getSrcDest()); break;
                case 0xA00: ldt(address, inst.getSrcDest()); break;
                case 0xA20: stt(inst.getSrcDest(), address); break;
                case 0xB00: ldq(address, inst.getSrcDest()); break;
                case 0xB20: stq(inst.getSrcDest(), address); break;
                case 0xC00: ldib(address, inst.getSrcDest()); break;
                case 0xC20: stib(inst.getSrcDest(), address); break;
                case 0xC80: ldis(address, inst.getSrcDest()); break;
                case 0xCA0: stis(inst.getSrcDest(), address); break;
                default:
                    raiseFault();
                    break;
            }
        }
        void
        execute(const COBRInstruction &inst) noexcept {
            switch (inst.getOpcode()) {
                case 0x300: bbc(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x370: bbs(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x3A0: cmpibe(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x3D0: cmpibne(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x3C0: cmpibl(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x3E0: cmpible(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x390: cmpibg(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x3B0: cmpibge(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x3F0: cmpibo(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x380: cmpibno(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x320: cmpobe(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x350: cmpobne(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x340: cmpobl(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x360: cmpoble(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x310: cmpobg(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x330: cmpobge(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                case 0x220: teste(inst.getSrc1()); break;
                case 0x250: testne(inst.getSrc1()); break;
                case 0x240: testl(inst.getSrc1()); break;
                case 0x260: testle(inst.getSrc1()); break;
                case 0x210: testg(inst.getSrc1()); break;
                case 0x230: testge(inst.getSrc1()); break;
                case 0x270: testo(inst.getSrc1()); break;
                case 0x200: testno(inst.getSrc1()); break;
                default:
                    raiseFault();
                    break;
            }
        }
        void
        execute(const CTRLInstruction &inst) noexcept {
            switch (inst.getOpcode()) {
                case 0x080: b(inst.getDisplacement()); break;
                case 0x090: call(inst.getDisplacement()); break;
                case 0x0A0: ret(); break;
                case 0x0B0: bal(Displacement22{inst.getDisplacement()}); break;
                case 0x100: bno(Displacement22{inst.getDisplacement()}); break;
                case 0x110: bg(Displacement22{inst.getDisplacement()}); break;
                case 0x120: be(Displacement22{inst.getDisplacement()}); break;
                case 0x130: bge(Displacement22{inst.getDisplacement()}); break;
                case 0x140: bl(Displacement22{inst.getDisplacement()}); break;
                case 0x150: bne(Displacement22{inst.getDisplacement()}); break;
                case 0x160: ble(Displacement22{inst.getDisplacement()}); break;
                case 0x170: bo(Displacement22{inst.getDisplacement()}); break;
                case 0x180: faultno(); break;
                case 0x190: faultg(); break;
                case 0x1A0: faulte(); break;
                case 0x1B0: faultge(); break;
                case 0x1C0: faultl(); break;
                case 0x1D0: faultne(); break;
                case 0x1E0: faultle(); break;
                case 0x1F0: faulto(); break;
                default:
                    raiseFault();
                    break;
            }
        }
    private: // fault operations
        void
        faultno() {
            if (ac.getConditionCode() == 0) {
                raiseFault();
            }
        }
        void
        faultg() {
            if (ac.conditionIsGreaterThan()) {
                raiseFault();
            }
        }
        void
        faultge() {
            if (ac.conditionIsGreaterThanOrEqualTo()) {
                raiseFault();
            }
        }
        void
        faultl() {
            if (ac.conditionIsLessThan()) {
                raiseFault();
            }
        }
        void
        faultle() {
            if (ac.conditionIsLessThanOrEqual()) {
                raiseFault();
            }
        }
        void
        faulte() {
            if (ac.conditionIsEqualTo()) {
                raiseFault();
            }
        }
        void
        faultne() {
            if (ac.conditionIsNotEqual()) {
                raiseFault();
            }
        }
        void
        faulto() {
            if (ac.conditionIsOrdered()) {
                raiseFault();
            }
        }

    private: // common internal functions
        void
        saveLocals() noexcept {
            // okay, we have to save all of the registers to the stack or the on board
            // register cache (however, I'm not implementing that yet)
        }
        void
        restoreLocals() noexcept {
            // restore the local register frame, generally done when you return from a
            // previous function
        }
        bool
        getCarryFlag() const noexcept {
            return ac.carryFlagSet();
        }
        void
        setCarryFlag(bool value) noexcept {
            ac.setCarryFlag(value);
        }
    private: // data movement operations
        void
        lda(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setOrdinal(mem);
        }
        void
        ld(Ordinal address, RegisterIndex dest) {
            getRegister(dest).setOrdinal(loadOrdinal(address));
        }
        void
        ldob(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setByteOrdinal(loadByteOrdinal(mem));
        }

        void
        ldos(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setShortOrdinal(loadShortOrdinal(mem));
        }

        void
        ldib(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setByteInteger(loadByteInteger(mem));
        }

        void
        ldis(Ordinal mem, RegisterIndex dest) {
            getRegister(dest).setShortInteger(loadShortInteger(mem));
        }

        void
        st(RegisterIndex src, Ordinal dest) {
            storeOrdinal(dest, getRegister(src).getOrdinal());
        }

        void
        stob(RegisterIndex src, Ordinal dest) {
            storeByteOrdinal(dest, getRegister(src).getByteOrdinal());
        }

        void
        stib(RegisterIndex src, Ordinal dest) {
            storeByteInteger(dest, getRegister(src).getByteInteger());
        }

        void
        stis(RegisterIndex src, Ordinal dest) {
            storeShortInteger(dest, getRegister(src).getShortInteger());
        }

        void
        stos(RegisterIndex src, Ordinal dest) {
            storeShortOrdinal(dest, getRegister(src).getShortOrdinal());
        }

        void
        stl(RegisterIndex src, Ordinal address) {
            if (!divisibleByTwo(src)) {
                /// @todo raise a operation.invalid_operand fault
                raiseFault();
            } else {
                storeOrdinal(address, getRegister(src).getOrdinal());
                storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
                if ((address & 0b111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED fault
                    raiseFault();
                }
            }
        }
        void
        stt(RegisterIndex src, Ordinal address) {
            if (!divisibleByFour(src)) {
                /// @todo raise a operation.invalid_operand fault
                raiseFault();
            } else {
                storeOrdinal(address, getRegister(src).getOrdinal());
                storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
                storeOrdinal(address + 8, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
                if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }

        void
        stq(RegisterIndex src, Ordinal address) {
            if (!divisibleByFour(src)) {
                raiseFault();
                /// @todo raise a operation.invalid_operand fault
            } else {
                storeOrdinal(address, getRegister(src).getOrdinal());
                storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
                storeOrdinal(address + 8, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
                storeOrdinal(address + 12, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
                if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                    raiseFault();
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                }
            }
        }

        void
        ldl(Ordinal mem, RegisterIndex dest) {
            if(!divisibleByTwo(dest)) {
                /// @todo raise invalid_operand fault
                // the Hx docs state that dest is modified
                getRegister(dest).setOrdinal(-1);
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(loadOrdinal(mem));
                getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
                if ((mem & 0b111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }
        void
        ldt(Ordinal mem, RegisterIndex dest) {
            if(!divisibleByFour(dest)) {
                /// @todo raise invalid_operand fault
                // the Hx docs state that dest is modified
                getRegister(dest).setOrdinal(-1);
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(loadOrdinal(mem));
                getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem+8));
                if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }
        void
        ldq(Ordinal mem, RegisterIndex dest) {
            if(!divisibleByFour(dest)) {
                /// @todo raise invalid_operand fault
                // the Hx docs state that dest is modified
                getRegister(dest).setOrdinal(-1);
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(loadOrdinal(mem));
                getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem+8));
                getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(loadOrdinal(mem+12));
                if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                    /// @todo generate an OPERATION.UNALIGNED_FAULT
                    raiseFault();
                }
            }
        }

        void
        mov(RegLit src, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
        }
        void
        movl(RegLit src, RegisterIndex dest) {
            // so this is a bit of a hack but according to the i960Hx manual only the least significant register gets the literal
            if (!divisibleByTwo(dest) || (isRegisterIndex(src) && !divisibleByTwo(std::get<RegisterIndex>(src)))) {
                getRegister(dest).setInteger(-1);
                getRegister(nextRegisterIndex(dest)).setInteger(-1);
                /// @todo generate a fault here!
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
            }
        }
        void
        movt(RegLit src, RegisterIndex dest) {
            if (!divisibleByFour(dest) || (isRegisterIndex(src) && !divisibleByFour(std::get<RegisterIndex>(src)))) {
                getRegister(dest).setInteger(-1);
                getRegister(nextRegisterIndex(dest)).setInteger(-1);
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setInteger(-1);
                /// @todo generate a fault here!
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(extractValue(nextValue(nextValue(src)), TreatAsOrdinal{}));
            }
        }
        void
        movq(RegLit src, RegisterIndex dest) {
            if (!divisibleByFour(dest) || (isRegisterIndex(src) && !divisibleByFour(std::get<RegisterIndex>(src)))) {
                getRegister(dest).setInteger(-1);
                getRegister(nextRegisterIndex(dest)).setInteger(-1);
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setInteger(-1);
                getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setInteger(-1);
                /// @todo generate a fault here!
                raiseFault();
            } else {
                getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(extractValue(nextValue(nextValue(src)), TreatAsOrdinal{}));
                getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(extractValue(nextValue(nextValue(nextValue(src))), TreatAsOrdinal{}));
            }
        }

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
        void
        addc(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
            auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
            auto c = getCarryFlag() ? 1 : 0;
            auto result = s2 + s1 + c;
            auto upperHalf = static_cast<Ordinal>(result >> 32);
            setCarryFlag(upperHalf != 0) ;
            getRegister(dest).setOrdinal(static_cast<Ordinal>(result));
            /// @todo check for integer overflow condition
        }
        void
        addi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 + s1) ;
            /// @todo implement fault detection
        }

        void
        addo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 + s1) ;
            /// @todo implement fault detection
        }
        void
        subi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 - s1) ;
            /// @todo implement fault detection
        }
        void
        subo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 - s1) ;
            /// @todo implement fault detection
        }
        void
        subc(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
            auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
            auto c = getCarryFlag() ? 1 : 0;
            auto result = s2 - s1 + c;
            auto upperHalf = static_cast<Ordinal>(result >> 32);
            setCarryFlag(upperHalf != 0);
            /// @todo do integer overflow subtraction check
            getRegister(dest).setOrdinal(static_cast<Ordinal>(result));
        }
        void
        muli(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 * s1) ;
            /// @todo implement fault detection
        }
        void
        mulo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 * s1) ;
            /// @todo implement fault detection
        }
        void
        divi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsInteger{}) ;
            auto s2 = extractValue(src2, TreatAsInteger{}) ;
            getRegister(dest).setInteger(s2 / s1) ;
            /// @todo implement fault detection
        }
        void
        divo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
            auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
            getRegister(dest).setOrdinal(s2 / s1) ;
            /// @todo implement fault detection
        }
        void
        eshro(RegLit src1, RegLit src2, RegisterIndex dest) {
            /// @todo implement
        }

        void
        ediv(RegLit src1, RegLit src2, RegisterIndex dest) {
            /// @todo implement
        }

        void
        emul(RegLit src1, RegLit src2, RegisterIndex dest) {
            /// @todo implement
        }
        void
        remi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s2 = extractValue(src2, TreatAsInteger{});
            auto s1 = extractValue(src1, TreatAsInteger{});
            getRegister(dest).setInteger(((s2 / s1) * s1));
        }
        void
        remo(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            auto s1 = extractValue(src1, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(((s2 / s1) * s1));
        }
        void
        modi(RegLit src1, RegLit src2, RegisterIndex dest) {
            // taken from the manual
            auto denominator = extractValue(src1, TreatAsInteger{});
            auto numerator = extractValue(src2, TreatAsInteger{});
            if (denominator == 0) {
                // @todo raise Arithmetic Zero Divide fault
                raiseFault();
                return;
            }
            auto theDestValue = numerator - ((numerator / denominator) * denominator);
            auto& dReg = getRegister(dest);
            dReg.setInteger(theDestValue);
            if (((numerator * denominator) < 0) && (theDestValue != 0)) {
                dReg.setInteger(theDestValue + denominator);
            }
        }
        void
        shlo(RegLit len, RegLit src, RegisterIndex dest) {
            auto theLength = extractValue(len, TreatAsOrdinal{});
            auto theSrc = extractValue(src, TreatAsOrdinal{});
            if (theLength < 32) {
                getRegister(dest).setOrdinal(theSrc << theLength);
            } else {
                getRegister(dest).setOrdinal(0);
            }
        }
        void
        shro(RegLit len, RegLit src, RegisterIndex dest) {
            auto theLength = extractValue(len, TreatAsOrdinal{});
            auto theSrc = extractValue(src, TreatAsOrdinal{});
            if (theLength < 32) {
                getRegister(dest).setOrdinal(theSrc >> theLength);
            } else {
                getRegister(dest).setOrdinal(0);
            }
        }
        void
        shli(RegLit len, RegLit src, RegisterIndex dest) {
            auto theLength = extractValue(len, TreatAsInteger{});
            auto theSrc = extractValue(src, TreatAsInteger{});
            getRegister(dest).setInteger(theSrc << theLength);
        }
        /// @todo correctly implement shri and shrdi
        void
        shri(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto src = extractValue(src2, TreatAsInteger{});
            auto len = abs(extractValue(src1, TreatAsInteger{}));
            if (len > 32)  {
                len = 32;
            }
            getRegister(dest).setInteger(src >> len);
        }

        void
        shrdi(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto src = extractValue(src2, TreatAsInteger{});
            auto len = abs(extractValue(src1, TreatAsInteger{}));
            auto result = src >> len;
            if (src < 0 && result < 0) {
                ++result;
            }
            getRegister(dest).setInteger(result);
        }
        void
        rotate(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto len = extractValue(src1, TreatAsOrdinal {});
            auto src = extractValue(src2, TreatAsOrdinal {});
            getRegister(dest).setOrdinal(rotateOperation(src, len));
        }

    private: // logical operations
        void
        logicalAnd(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(
                    extractValue(src2, TreatAsOrdinal{}) &
                    extractValue(src1, TreatAsOrdinal{}));
        }
        void
        andnot(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(
                    (extractValue(src2, TreatAsOrdinal{})) &
                    (~extractValue(src1, TreatAsOrdinal{})));
        }
        void
        logicalNand(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | (~extractValue(src1, TreatAsOrdinal{})));
        }

        void
        logicalNor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & (~extractValue(src1, TreatAsOrdinal{})));
        }

        void
        logicalNot(RegLit src, RegisterIndex dest) {
            getRegister(dest).setOrdinal(~extractValue(src, TreatAsOrdinal{}));
        }
        void
        notand(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & extractValue(src1,TreatAsOrdinal{}));
        }
        void
        notor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | extractValue(src1,TreatAsOrdinal{}));
        }
        void
        logicalOr(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | extractValue(src1,TreatAsOrdinal{}));
        }
        void
        ornot(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | (~extractValue(src1,TreatAsOrdinal{})));
        }
        void
        logicalXor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{}));
        }
        void
        logicalXnor(RegLit src1, RegLit src2, RegisterIndex dest) {
            getRegister(dest).setOrdinal(~(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{})));
        }
    private: // bit and bit-field operations
        void
        setbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{}) ;
            auto src = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
        }
        void
        clrbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{});
            auto src = extractValue(src2, TreatAsOrdinal{});
            auto bitposModified = ~(computeSingleBitShiftMask(bitpos));
            getRegister(dest).setOrdinal(src & bitposModified);
        }
        void
        notbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{});
            auto src = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(src ^ (1 << (bitpos & 0b11111)));
        }

        void
        alterbit(RegLit src1, RegLit src2, RegisterIndex dest) {
            auto bitpos = extractValue(src1, TreatAsOrdinal{});
            auto src = extractValue(src2, TreatAsOrdinal{});
            if ((ac.getConditionCode() & 0b010) == 0) {
                getRegister(dest).setOrdinal(src & (~(1 << (bitpos & 0b11111))));
            } else {
                getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
            }
        }
        void
        chkbit(RegLit src1, RegLit src2) {
            ac.setConditionCode(((extractValue(src2, TreatAsOrdinal{})& computeSingleBitShiftMask(extractValue(src1, TreatAsOrdinal{}))) == 0) ? 0b000 : 0b010);
        }
        static constexpr Ordinal largestOrdinal = 0xFFFF'FFFF;
        void
        spanbit(RegLit src1, RegisterIndex dest) {
            /**
             * Find the most significant clear bit
             */
            auto result = largestOrdinal;
            ac.clearConditionCode();
            if (auto src = extractValue(src1, TreatAsOrdinal{}); src != largestOrdinal) {
                for (Integer i = 31; i >= 0; --i) {
                    if (auto k = (1 << i); (src & k) == 0) {
                        result = i;
                        ac.setConditionCode(0b010);
                        break;
                    }
                }
            }
            getRegister(dest).setOrdinal(result);
        }
        void
        scanbit(RegLit src, RegisterIndex dest) {
            // find the most significant set bit
            auto result = largestOrdinal;
            ac.clearConditionCode();
            // while the psuedo-code in the programmers manual talks about setting
            // the destination to all ones if src is equal to zero, there is no short
            // circuit in the action section for not iterating through the loop when
            // src is zero. A small optimization
            if (auto theSrc = extractValue(src, TreatAsOrdinal{}); theSrc != 0) {
                for (Integer i = 31; i >= 0; --i) {
                    if (auto k = 1 << i; (theSrc & k) != 0) {
                        ac.setConditionCode(0b010);
                        result = i;
                        break;
                    }
                }
            }
            getRegister(dest).setOrdinal(result);
        }
        void
        extract(RegLit src1, RegLit src2, RegisterIndex dest) {
            // taken from the i960Hx manual
            getRegister(dest).setOrdinal((extractValue(dest, TreatAsOrdinal{}) >> std::min(extractValue(src1, TreatAsOrdinal{}), static_cast<Ordinal>(32))) &
                                         (~(0xFFFF'FFFF << extractValue(src2, TreatAsOrdinal{}))));
        }
        void
        modify(RegLit mask, RegLit src, RegisterIndex srcDest) {
            auto& sd = getRegister(srcDest);
            auto theMask = extractValue(mask, TreatAsOrdinal{});
            sd.setOrdinal((extractValue(src, TreatAsOrdinal{}) & theMask)  | (sd.getOrdinal() & (~theMask)));
        }

        void
        scanbyte(RegLit src1, RegLit src2) {
            ac.clearConditionCode();
            if (auto s1 = extractValue(src1, TreatAsOrdinal{}), s2 = extractValue(src2, TreatAsOrdinal{});
                    ((s1 & 0x0000'00FF) == (s2 & 0x0000'00FF)) ||
                    ((s1 & 0x0000'FF00) == (s2 & 0x0000'FF00)) ||
                    ((s1 & 0x00FF'0000) == (s2 & 0x00FF'0000)) ||
                    ((s1 & 0xFF00'0000) == (s2 & 0xFF00'0000))) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b000);
            }
        }
    private: // compare and increment or decrement
        void
        cmpo(RegLit src1, RegLit src2) {
            auto s1 = extractValue(src1, TreatAsOrdinal{});
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            if (s1 < s2) {
                ac.setConditionCode(0b100);
            } else if (s1 == s2) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b001);
            }
        }
        void
        cmpi(RegLit src1, RegLit src2) {
            auto s1 = extractValue(src1, TreatAsInteger{});
            auto s2 = extractValue(src2, TreatAsInteger{});
            if (s1 < s2) {
                ac.setConditionCode(0b100);
            } else if (s1 == s2) {
                ac.setConditionCode(0b010);
            } else {
                ac.setConditionCode(0b001);
            }
        }
        void
        concmpo(RegLit src1, RegLit src2) {
            // don't care what the least significant two bits are of the cond code so just mask them out
            if ((ac.getConditionCode() & 0b100) == 0) {
                auto s1 = extractValue(src1, TreatAsOrdinal{});
                auto s2 = extractValue(src2, TreatAsOrdinal{});
                ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
            }
        }
        void
        concmpi(RegLit src1, RegLit src2) {
            // don't care what the least significant two bits are of the cond code so just mask them out
            if ((ac.getConditionCode() & 0b100) == 0) {
                auto s1 = extractValue(src1, TreatAsInteger{});
                auto s2 = extractValue(src2, TreatAsInteger{});
                ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
            }

        }
        void
        cmpinco(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpo(src1, src2);
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(s2 + 1);
        }
        void
        cmpinci(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpi(src1, src2);
            auto s2 = extractValue(src2, TreatAsInteger{});
            getRegister(dest).setInteger(s2 + 1); // manual states that this instruction suppresses overflow
        }
        void
        cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpo(src1, src2);
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            getRegister(dest).setOrdinal(s2 - 1);
        }
        void
        cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest) {
            cmpi(src1, src2);
            auto s2 = extractValue(src2, TreatAsInteger{});
            getRegister(dest).setInteger(s2 - 1); // manual states that this instruction suppresses overflow
        }
    private: // branching
        /// @todo figure out correct signatures
        void
        b(Displacement22 targ) {
            ip.setInteger(ip.getInteger() + targ.getValue());
        }
        void
        bal(Displacement22 targ) {
            globals[14].setOrdinal(ip.getOrdinal() + 4);
            // make sure that the code is consistent
            b(targ);
        }
        void
        bx(Ordinal targ) {
            ip.setOrdinal(targ);
        }
        void
        balx(Ordinal targ, RegisterIndex dest) {
            getRegister(dest).setOrdinal(ip.getOrdinal());
            ip.setOrdinal(targ);
        }

        /// @todo condense this operation set down to a single function since the mask is embedded in the opcode itself :)
        void
        bno(Displacement22 dest) {
            if (ac.conditionIsUnordered()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        void
        bo(Displacement22 dest) {
            if (ac.conditionIsOrdered()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        void
        bg(Displacement22 dest) {
            if (ac.conditionIsGreaterThan()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        void
        be(Displacement22 dest) {
            if (ac.conditionIsEqualTo()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }

        void
        bge(Displacement22 dest) {
            if (ac.conditionIsGreaterThanOrEqualTo()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        void
        bl(Displacement22 dest) {
            if (ac.conditionIsLessThan()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        void
        bne(Displacement22 dest) {
            if (ac.conditionIsNotEqual()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
        void
        ble(Displacement22 dest) {
            if (ac.conditionIsLessThanOrEqual()) {
                ip.setInteger(ip.getInteger() + dest.getValue());
            }
        }
    private: // compare and branch
        /// @todo figure out correct signatures
        void
        cmpibg(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpi(src1, src2);
            bg(Displacement22{targ});
        }

        void
        cmpible(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpi(src1, src2);
            ble(Displacement22{targ});

        }

        void
        cmpibe(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpi(src1, src2);
            be(Displacement22{targ});

        }

        void
        cmpibne(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpi(src1, src2);
            bne(Displacement22{targ});
        }

        void
        cmpibl(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpi(src1, src2);
            bl(Displacement22{targ});
        }
        void
        cmpibge(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpi(src1, src2);
            bge(Displacement22{targ});
        }
        void
        cmpobg(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpo(src1, src2);
            bg(Displacement22{targ});
        }

        void
        cmpoble(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpo(src1, src2);
            ble(Displacement22{targ});
        }

        void
        cmpobe(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpo(src1, src2);
            be(Displacement22{targ});
        }

        void
        cmpobne(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpo(src1, src2);
            bne(Displacement22{targ});
        }

        void
        cmpobl(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpo(src1, src2);
            bl(Displacement22{targ});
        }
        void
        cmpobge(RegLit src1, RegisterIndex src2, ShortInteger targ) {
            cmpo(src1, src2);
            bge(Displacement22{targ});
        }
        void
        bbc(RegLit bitpos, RegisterIndex src, ShortInteger targ) {
            auto bpos = extractValue(bitpos, TreatAsOrdinal{});
            auto theSrc = getRegister(src).getOrdinal();
            auto theMask = computeSingleBitShiftMask(bpos);
            constexpr Ordinal startingConditionCode = 0b010;
            constexpr Ordinal onConditionMet = 0b000;
            constexpr Ordinal compareAgainst = 0;
            ac.setConditionCode(startingConditionCode);
            if ((theSrc & theMask) == compareAgainst) {
                ac.setConditionCode(onConditionMet);
                ip.setInteger(ip.getInteger() + targ);
            }
        }
        void
        bbs(RegLit bitpos, RegisterIndex src, ShortInteger targ) {
            auto bpos = extractValue(bitpos, TreatAsOrdinal{});
            auto theSrc = getRegister(src).getOrdinal();
            auto theMask = computeSingleBitShiftMask(bpos);
            constexpr Ordinal startingConditionCode = 0b000;
            constexpr Ordinal onConditionMet = 0b010;
            constexpr Ordinal compareAgainst = 1;
            ac.setConditionCode(startingConditionCode);
            if ((theSrc & theMask) == compareAgainst) {
                ac.setConditionCode(onConditionMet);
                ip.setInteger(ip.getInteger() + targ);
            }
        }
        void
        cmpibo(RegLit src1, RegLit src2, ShortInteger targ) {
            cmpi(src1, src2);
            bo(Displacement22{targ});
        }
        void
        cmpibno(RegLit src1, RegLit src2, ShortInteger targ) {
            cmpi(src1, src2);
            bno(Displacement22{targ});
        }
    private: // test condition codes
        void
        testo(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsOrdered() ? 1 : 0);
        }
        void
        testno(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.getConditionCode() == 0 ? 1 : 0);
        }
        void
        teste(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsEqualTo() ? 1 : 0);
        }
        void
        testne(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsNotEqual() ? 1 : 0);
        }
        void
        testl(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsLessThan() ? 1 : 0);
        }
        void
        testle(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsLessThanOrEqual() ? 1 : 0);
        }
        void
        testg(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsGreaterThan() ? 1 : 0);
        }

        void
        testge(RegisterIndex dest) {
            getRegister(dest).setOrdinal(ac.conditionIsGreaterThanOrEqualTo() ? 1 : 0);
        }
    private:
        static constexpr RegisterIndex PFP = static_cast<RegisterIndex>(0b00000);
        static constexpr RegisterIndex SP = static_cast<RegisterIndex>(0b00001);
        static constexpr RegisterIndex RIP = static_cast<RegisterIndex>(0b00010);
        static constexpr RegisterIndex FP = static_cast<RegisterIndex>(0b11111);
        Ordinal
        getStackPointerAddress() const noexcept {
            return getRegister(SP).getOrdinal();
        }
        void
        setRIP(const Register& ip) noexcept {
            getRegister(RIP).setOrdinal(ip.getOrdinal());
        }
        Ordinal
        getFramePointerAddress() const noexcept {
            return getRegister(FP).getOrdinal();
        }
        void
        setPFP(Ordinal value) noexcept {
            getRegister(PFP).setOrdinal(value);
        }
        void
        setFramePointer(Ordinal value) noexcept {
            getRegister(FP).setOrdinal(value);
        }
        void
        setStackPointer(Ordinal value) noexcept {
            getRegister(SP).setOrdinal(value);
        }
        void
        allocateNewLocalRegisterSet() {
            /// @todo implement at some point
        }
    private: // call and return (note, no supervisor mode right now)
        /// @todo figure out correct signatures
        void
        call(Displacement22 targ) {
            auto newAddress = targ.getValue();
            // the and operation clears out the least significant N bits of this new address
            // make a new stack frame
            auto tmp = (getStackPointerAddress() + computeAlignmentBoundaryConstant()) &
                    (~computeAlignmentBoundaryConstant());
            setRIP(ip);
            saveLocals();
            allocateNewLocalRegisterSet();
            auto addr = ip.getInteger();
            ip.setInteger(addr + newAddress);
            setPFP(getFramePointerAddress());
            setFramePointer(tmp);
            setStackPointer(tmp + 64);
        }
        void callx(Ordinal targ) {

            // the and operation clears out the least significant N bits of this new address
            // make a new stack frame
            auto tmp = (getStackPointerAddress() + computeAlignmentBoundaryConstant()) &
                       (~computeAlignmentBoundaryConstant());
            setRIP(ip);
            saveLocals();
            allocateNewLocalRegisterSet();
            ip.setOrdinal(targ);
            setPFP(getFramePointerAddress());
            setFramePointer(tmp);
            setStackPointer(tmp + 64);
        }
        void calls(RegLit targ) {
            /// @todo implement
        }
        void ret() {
            syncf();
            /// @todo continue implementing
        }
        /// @todo implement faults as exceptions
    private: // processor management
        void
        flushreg() {
            // noop right now
        }
        void
        modtc(const RegFormatInstruction& inst) {
            /// @todo implement
        }
        void
        modpc(const RegFormatInstruction& inst) {
            /// @todo implement
        }
        void
        modac(const RegFormatInstruction &inst) {
            // in this case, mask is src/dst
            // src is src2
            // dest is src1
            auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
            auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
            auto dest = std::visit([](auto&& value) -> RegisterIndex {
                using K = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<K, RegisterIndex>) {
                    return value;
                } else if constexpr (std::is_same_v<K, Literal>) {
                    return toRegisterIndex(toInteger(value));
                } else {
                    static_assert(DependentFalse<K>, "Unresolved type!");
                }
            }, inst.getSrc1());
            auto tmp = ac.getRawValue();
            ac.setRawValue((src & mask) | (tmp & (~mask)));
            getRegister(dest).setOrdinal(tmp);
        }
        void
        syncf() {
            if (ac.getNoImpreciseFaults()) {
                return;
            }
            // do a noop
        }
    private: // marking
        void
        fmark() {
            /// @todo implement
        }
        void
        mark() {
            /// @todo implement
        }
    private: // Numerics Architecture addons
        void
        dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
            /// @todo implement... such a baffling design...BCD...
        }
        void
        dmovt(RegisterIndex src1, RegisterIndex dest) {
            auto srcValue = extractValue(src1, TreatAsOrdinal { });
            getRegister(dest).setOrdinal(srcValue);
            auto lowest8 = static_cast<ByteOrdinal>(srcValue);
            ac.setConditionCode(((lowest8 >= 0b0011'0000) && (lowest8 <= 0b0011'1001)) ? 0b000 : 0b010);
        }
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
        void
        daddc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
            const auto& s1 = getRegister(src1);
            const auto& s2 = getRegister(src2);
            auto& dst = getRegister(dest);
            // transfer bits over
            dst.setOrdinal(s2.getOrdinal());
            auto outcome = (s2.getByteOrdinal() & 0xF) + (s1.getByteOrdinal() & 0xF) + (getCarryFlag() ? 1 : 0);
            setCarryFlag((outcome & 0xF0) != 0);
            dst.setByteOrdinal((s2.getByteOrdinal() & 0xF0) | (outcome & 0x0F));
        }

    private:
        RegisterFile globals, locals;
        Register ip; // always start at address zero
        ArithmeticControls ac;
        bool _unalignedFaultEnabled = false;
        unsigned int _salign = 1;
    };

}
#endif // end I960_CORE_H__

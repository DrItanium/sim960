//
// Created by jwscoggins on 12/5/20.
//
#include "ArithmeticControls.h"
#include "Core.h"
#include "PreviousFramePointer.h"
#include "ProcessorAddress.h"
#include "InstructionFormats.h"
#ifndef ARDUINO
#include <iostream>
#include <string>
#endif
namespace i960 {
#ifndef ARDUINO
    void
    displayInstruction(Ordinal address, const std::string& name) {
        std::cout << "\t\tExecuting: " << name << " @ 0x" << std::hex << address << std::endl;
    }
#define AnInstruction displayInstruction(ip.getOrdinal(), __PRETTY_FUNCTION__)
#else // !ARDUINO defined
#define AnInstruction
#endif
    constexpr Ordinal largestOrdinal = 0xFFFF'FFFF;
    constexpr RegisterIndex PFP = LocalRegister[0];
    constexpr RegisterIndex SP = LocalRegister[1];
    constexpr RegisterIndex RIP = LocalRegister[2];
    constexpr RegisterIndex g0 = GlobalRegister[0];
    constexpr RegisterIndex FP = GlobalRegister[15];
    Core::DecodedInstruction
    Core::decode(Ordinal lower, Ordinal upper) noexcept {
        if (auto opcode = static_cast<ByteOrdinal>(lower >> 24); opcode < 0x20) {
            return CTRLInstruction(lower);
        } else if (opcode >= 0x20 && opcode < 0x58) {
            return COBRInstruction(lower);
        } else if (opcode >= 0x58 && opcode < 0x80) {
            return RegFormatInstruction(lower);
        } else {
            return MEMFormatInstruction(lower, upper);
        }
    }
    RegLit
    Core::nextValue(RegLit value) const noexcept {
        return std::visit([this](auto &&value) -> RegLit {
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
    void
    Core::ediv(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        // a total copy and paste hack job but it will work
        if (std::holds_alternative<Literal>(src2)) {
            // this is a little different than normal
            auto denominator = extractValue(src1, TreatAsOrdinal{});
            if (!divisibleByTwo(dest)) {
                setRegister(dest, -1, TreatAsOrdinal{});
                setRegister(nextRegisterIndex(dest), -1, TreatAsOrdinal{});
                raiseFault(); // OPERATION.INVALID_OPERAND
            } else if (denominator == 0) {
                setRegister(dest, -1, TreatAsOrdinal{});
                setRegister(nextRegisterIndex(dest), -1, TreatAsOrdinal{});
                raiseFault(); // ARITHMETIC.DIVIDE_ZERO
            } else {
                auto numerator = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
                auto quotient = numerator / denominator;
                auto remainder = numerator - (numerator / denominator) * denominator;
                setRegister(nextRegisterIndex(dest), quotient, TreatAsOrdinal{});
                setRegister(dest, remainder, TreatAsOrdinal{});
            }
        } else {
            // okay we are holding onto a RegisterIndex in src2
            auto s2ri = std::get<RegisterIndex>(src2);
            auto denominator = extractValue(src1, TreatAsOrdinal{});
            if (!divisibleByTwo(s2ri) || !divisibleByTwo(dest)) {
                setRegister(dest, -1, TreatAsOrdinal{});
                setRegister(nextRegisterIndex(dest), -1, TreatAsOrdinal{});
                raiseFault(); // OPERATION.INVALID_OPERAND
            } else if (denominator == 0) {
                setRegister(dest, -1, TreatAsOrdinal{});
                setRegister(nextRegisterIndex(dest), -1, TreatAsOrdinal{});
                raiseFault(); // ARITHMETIC.DIVIDE_ZERO
            } else {
                LongRegister tmp(getRegister(s2ri),
                                 getRegister(nextRegisterIndex(s2ri)));
                auto numerator = tmp.getOrdinal();
                auto quotient = numerator / denominator;
                auto remainder = numerator - (numerator / denominator) * denominator;
                setRegister(nextRegisterIndex(dest), quotient, TreatAsOrdinal{});
                setRegister(dest, remainder, TreatAsOrdinal{});
            }
        }
    }

    void
    Core::emul(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        if (!divisibleByTwo(dest)) {
            setRegister(dest, -1, TreatAsOrdinal{});
            setRegister(nextRegisterIndex(dest), -1, TreatAsOrdinal{});
            raiseFault(); // OPERATION.INVALID_OPERAND
        } else {
            auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
            auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
            auto result = s1 * s2;
            setRegister(dest, result, TreatAsOrdinal {});
            setRegister(nextRegisterIndex(dest), static_cast<Ordinal>(result >> 32), TreatAsOrdinal {});
        }
    }
    void
    Core::execute(const RegFormatInstruction &inst) noexcept {
        auto theOpcode = inst.getOpcode();
        switch (theOpcode) {
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
            case 0x5B0: addc(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x5B2: subc(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                // case 0x5B4: intdis(); break;
                // case 0x5B5: inten(); break;
            case 0x5CC: mov(inst.getSrc1(), inst.getDestination()); break;
                //case 0x5D8: eshro(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
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
                case 0x784: selno(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x790: addog(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x791: addig(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x792: subog(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x793: subig(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x794: selg(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7A0: addoe(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7A1: addie(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7A2: suboe(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7A3: subie(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x7A4: sele(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7B0: addoge(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7B1: addige(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7B2: suboge(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7B3: subige(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x7B4: selge(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7C0: addol(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7C1: addil(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7C2: subol(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7C3: subil(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x7C4: sell(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7D0: addone(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7D1: addine(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7D2: subone(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7D3: subine(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x7D4: selne(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7E0: addole(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7E1: addile(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7E2: subole(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7E3: subile(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x7E4: selle(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7F0: addoo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7F1: addio(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7F2: suboo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x7F3: subio(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                case 0x7F4: selo(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            default:
                badInstruction(inst);
                /// @todo raise an error at this point
                break;
        }
    }
    void
    Core::execute(const MEMFormatInstruction &inst) noexcept {
        auto address = inst.computeAddress(*this);
        auto opcode = inst.getOpcode();
        switch (opcode) {
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
            case 0x9A0: stl(inst.getSrcDest(), address); break;
            case 0xA00: ldt(address, inst.getSrcDest()); break;
            case 0xA20: stt(inst.getSrcDest(), address); break;
            case 0xB00: ldq(address, inst.getSrcDest()); break;
            case 0xB20: stq(inst.getSrcDest(), address); break;
            case 0xC00: ldib(address, inst.getSrcDest()); break;
            case 0xC20: stib(inst.getSrcDest(), address); break;
            case 0xC80: ldis(address, inst.getSrcDest()); break;
            case 0xCA0: stis(inst.getSrcDest(), address); break;
            default: badInstruction(inst); break;
        }
    }
    void
    Core::cmpobg(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::GreaterThan>(src1, src2, targ, TreatAsOrdinal{});
    }
    void
    Core::cmpobe(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::EqualTo>(src1, src2, targ, TreatAsOrdinal{});
    }
    void
    Core::cmpobge(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::GreaterThanOrEqualTo>(src1, src2, targ, TreatAsOrdinal{});
    }
    void
    Core::cmpobl(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::LessThan>(src1, src2, targ, TreatAsOrdinal{});
    }
    void
    Core::cmpobne(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::NotEqual>(src1, src2, targ, TreatAsOrdinal{});
    }
    void
    Core::cmpoble(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::LessThanOrEqual>(src1, src2, targ, TreatAsOrdinal{});
    }
    void
    Core::cmpibg(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::GreaterThan>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpibe(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::EqualTo>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpibge(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::GreaterThanOrEqualTo>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpibl(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::LessThan>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpibne(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::NotEqual>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpible(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::LessThanOrEqual>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpibo(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::Ordered>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::cmpibno(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        AnInstruction;
        compareAndBranch<ConditionCodeKind::Unordered>(src1, src2, targ, TreatAsInteger{});
    }
    void
    Core::execute(const COBRInstruction &inst) noexcept {
        switch (inst.getOpcode()) {
            case 0x200: testBase<ConditionCodeKind::Unordered>(inst.getSrc1()); break;
            case 0x210: testBase<ConditionCodeKind::GreaterThan>(inst.getSrc1()); break;
            case 0x220: testBase<ConditionCodeKind::EqualTo>(inst.getSrc1()); break;
            case 0x230: testBase<ConditionCodeKind::GreaterThanOrEqualTo>(inst.getSrc1()); break;
            case 0x240: testBase<ConditionCodeKind::LessThan>(inst.getSrc1()); break;
            case 0x250: testBase<ConditionCodeKind::NotEqual>(inst.getSrc1()); break;
            case 0x260: testBase<ConditionCodeKind::LessThanOrEqual>(inst.getSrc1()); break;
            case 0x270: testBase<ConditionCodeKind::Ordered>(inst.getSrc1()); break;
            case 0x300: bbc(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x310: cmpobg(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x320: cmpobe(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x330: cmpobge(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x340: cmpobl(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x350: cmpobne(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x360: cmpoble(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x370: bbs(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x380: cmpibno(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x390: cmpibg(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3A0: cmpibe(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3B0: cmpibge(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3C0: cmpibl(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3D0: cmpibne(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3E0: cmpible(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3F0: cmpibo(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            default: badInstruction(inst); break;
        }
    }
    void
    Core::execute(const CTRLInstruction &inst) noexcept {
        switch (inst.getOpcode()) {
            case 0x080: b(inst.getDisplacement()); break;
            case 0x090: call(inst.getDisplacement()); break;
            case 0x0A0: ret(); break;
            case 0x0B0: bal(Displacement22{inst.getDisplacement()}); break;
            case 0x100: conditionalBranch<ConditionCodeKind::Unordered>(Displacement22{inst.getDisplacement()}); break;
            case 0x110: conditionalBranch<ConditionCodeKind::GreaterThan>(Displacement22{inst.getDisplacement()}); break;
            case 0x120: conditionalBranch<ConditionCodeKind::EqualTo>(Displacement22{inst.getDisplacement()}); break;
            case 0x130: conditionalBranch<ConditionCodeKind::GreaterThanOrEqualTo>(Displacement22{inst.getDisplacement()}); break;
            case 0x140: conditionalBranch<ConditionCodeKind::LessThan>(Displacement22{inst.getDisplacement()}); break;
            case 0x150: conditionalBranch<ConditionCodeKind::NotEqual>(Displacement22{inst.getDisplacement()}); break;
            case 0x160: conditionalBranch<ConditionCodeKind::LessThanOrEqual>(Displacement22{inst.getDisplacement()}); break;
            case 0x170: conditionalBranch<ConditionCodeKind::Ordered>(Displacement22{inst.getDisplacement()}); break;
            case 0x180: conditionalFault<ConditionCodeKind::Unordered>(); break;
            case 0x190: conditionalFault<ConditionCodeKind::GreaterThan>(); break;
            case 0x1A0: conditionalFault<ConditionCodeKind::EqualTo>(); break;
            case 0x1B0: conditionalFault<ConditionCodeKind::GreaterThanOrEqualTo>(); break;
            case 0x1C0: conditionalFault<ConditionCodeKind::LessThan>(); break;
            case 0x1D0: conditionalFault<ConditionCodeKind::NotEqual>(); break;
            case 0x1E0: conditionalFault<ConditionCodeKind::LessThanOrEqual>(); break;
            case 0x1F0: conditionalFault<ConditionCodeKind::Ordered>(); break;
            default: badInstruction(inst); break;
        }
    }
    void
    Core::cycle() {
        // always load two words
        auto lower = load(ip.getOrdinal());
        auto upper = load(ip.getOrdinal() + 4);
        cycle(lower, upper);
    }
    void
    Core::cycle(Ordinal lower, Ordinal upper) {
        executeInstruction(decode(lower, upper));
        ip.increment(ipIncrement_);
        // by default we increment by four so reset to that
        ipIncrement_ = 4;
    }
    void
    Core::executeInstruction(const DecodedInstruction &inst) {
        std::visit([this](auto &&theInst) { execute(theInst); }, inst);
    }
    void
    Core::raiseFault() {
        /// @todo implement
    }
    Register &
    Core::getRegister(RegisterIndex index) noexcept {
        if (auto offset = getOffset(index); isGlobalRegister(index)) {
            return globals[offset];
        } else {
            return locals[offset];
        }
    }
    const Register &
    Core::getRegister(RegisterIndex index) const noexcept {
        if (auto offset = getOffset(index); isGlobalRegister(index)) {
            return globals[offset];
        } else {
            return locals[offset];
        }
    }
    void
    Core::saveRegisterSet() noexcept {
        auto address = getFramePointer().getOrdinal();
        // okay, we have to save all of the registers to the stack or the on board
        // register cache (however, I'm not implementing that yet)
        for (const auto& c : locals) {
            store(address, c.getOrdinal());
            address +=4;
        }

    }
    void
    Core::restoreRegisterSet() noexcept {
        auto address = getFramePointer().getOrdinal();
        // restore the local register frame, generally done when you return from a
        // previous function
        for (auto & r : locals) {
            r.setOrdinal(load(address));
            address += 4;
        }
    }

    void
    Core::dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
        AnInstruction;
        const auto &s1 = getRegister(src1);
        const auto &s2 = getRegister(src2);
        // transfer bits over
        auto outcome = (s2.getOrdinal() - s1.getOrdinal() - 1 + (getCarryFlag() ? 1 : 0)) & 0xF;
        setCarryFlag(outcome != 0);
        setRegister(dest, (s2.getOrdinal() & (~0xF)) | (outcome & 0xF), TreatAsOrdinal {});
    }
    void
    Core::dmovt(RegisterIndex src1, RegisterIndex dest) {
        AnInstruction;
        auto srcValue = extractValue(src1, TreatAsOrdinal{});
        setRegister(dest, srcValue, TreatAsOrdinal{});
        auto lowest8 = static_cast<ByteOrdinal>(srcValue);
        ac.setConditionCode(((lowest8 >= 0b0011'0000) && (lowest8 <= 0b0011'1001)) ? 0b000 : 0b010);
    }
    void
    Core::daddc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
        AnInstruction;
        const auto &s1 = getRegister(src1);
        const auto &s2 = getRegister(src2);
        auto &dst = getRegister(dest);
        // transfer bits over
        dst.setOrdinal(s2.getOrdinal());
        auto outcome = (s2.getByteOrdinal() & 0xF) + (s1.getByteOrdinal() & 0xF) + (getCarryFlag() ? 1 : 0);
        setCarryFlag((outcome & 0xF0) != 0);
        dst.setByteOrdinal((s2.getByteOrdinal() & 0xF0) | (outcome & 0x0F));
    }
    void
    Core::fmark() {
        AnInstruction;
        if (pc.getTraceEnableBit()) {
            raiseFault(); // TRACE BREAKPOINT FAULT
        }
    }
    void
    Core::mark() {
        AnInstruction;
        if (pc.getTraceEnableBit() && tc.getMarkTraceMode()) {
            raiseFault(); // TRACE.MARK
        }
    }
    void
    Core::syncf() {
        AnInstruction;
        if (ac.getNoImpreciseFaults()) {
            return;
        }
        // do a noop
    }
    void
    Core::flushreg() {
        AnInstruction;
        // noop right now
    }
    constexpr RegisterIndex
    forceIntoRegisterIndex(RegLit value) noexcept {
        return std::visit([](auto &&value) -> RegisterIndex {
            using K = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<K, RegisterIndex>) {
                return value;
            } else if constexpr (std::is_same_v<K, Literal>) {
                return toRegisterIndex(toInteger(value));
            } else {
                static_assert(DependentFalse<K>, "Unresolved type!");
            }
        }, value);
    }
    void
    Core::modtc(const RegFormatInstruction &inst) {
        AnInstruction;
        auto dest = forceIntoRegisterIndex(inst.getSrc1());
        auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
        auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
        setRegister(dest, tc.modify(mask, src), TreatAsOrdinal{});
    }
    void
    Core::modpc(const RegFormatInstruction &inst) {
        AnInstruction;
        auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
        auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
        auto dest = forceIntoRegisterIndex(inst.getSrc1());
        auto tmp = pc.getRawValue();
        pc.setRawValue((src & mask) | (tmp & (~mask)));
        setRegister(dest, tmp, TreatAsOrdinal{});
    }
    void
    Core::modac(const RegFormatInstruction &inst) {
        AnInstruction;
        // in this case, mask is src/dst
        // src is src2
        // dest is src1
        auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
        auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
        auto dest = forceIntoRegisterIndex(inst.getSrc1());
        auto tmp = ac.getRawValue();
        ac.setRawValue((src & mask) | (tmp & (~mask)));
        setRegister(dest, tmp, TreatAsOrdinal{});
    }
    Ordinal
    Core::getSystemProcedureEntry(Ordinal targ) noexcept {
        return load((getSystemProcedureTableBase() + 0x30) + targ);
    }
    bool
    Core::registerSetAvailable() const noexcept {
        /// @todo update this when I implement proper register sets
        return false;
    }
    constexpr bool isSupervisorProcedure(Ordinal address) noexcept {
        return (address & 0b10);
    }
    constexpr bool isLocalProcedure(Ordinal address) noexcept {
        return !isSupervisorProcedure(address);
    }
    void
    Core::calls(RegLit targ) {
        AnInstruction;
        syncf();
        auto index = extractValue(targ, TreatAsOrdinal{});
        if (index > 259) {
            raiseFault(); // PROTECTION.LENGTH
        } else {
            auto temp = getSystemProcedureEntry(index);
            if (registerSetAvailable()) {
                allocateNewLocalRegisterSet();
            } else {
                saveRegisterSet();
                allocateNewLocalRegisterSet();
            }
            // save the next instruction location
            getReturnInstructionPointer().setOrdinal(ip.getOrdinal() + ipIncrement_);
            ip.setOrdinal(temp & (~0b11));
            Ordinal tempa = 0;
            if (isLocalProcedure(temp) || pc.inSupervisorMode()) {
                // Local call or supervisor call from supervisor mode
               tempa = (getStackPointer().getOrdinal() + (computeAlignmentBoundaryConstant())) & (~computeAlignmentBoundaryConstant());
               temp = 0;
            } else {
                tempa = getSupervisorStackPointer();
                temp = 0b010 | pc.getTraceEnableBit();
                pc.enterSupervisorMode();
                pc.setTraceEnableBit( temp & 0b1 );
            }
            auto pfp = getPFP();
            pfp.setRawValue(getFramePointer().getOrdinal());
            pfp.setReturnStatus(static_cast<PreviousFramePointer::ReturnStatusField>(temp));
            getFramePointer().setOrdinal(tempa);
            getStackPointer().setOrdinal(tempa + 64);
            doNotAdvanceIp();
        }
    }
    bool
    Core::registerSetNotAllocated(Ordinal address) const noexcept {
        /// @todo implement register set support at some point
        return true;
    }
    void
    Core::ret() {
        AnInstruction;
        syncf();
        auto pfp = getPFP();
        if (pfp.getPreReturnTraceFlag() && pc.getTraceEnableBit() && tc.getPrereturnTraceMode()) {
            pfp.setPreReturnTraceFlag(false);
            raiseFault(); // TRACE.PRERETURN
        } else {
            auto& fp = getFramePointer();
            Ordinal tempa = 0;
            Ordinal tempb = 0;
            auto getFPAndIP = [this, &fp, &pfp]() {
               fp.setOrdinal(pfp.getRawValue());
               freeCurrentRegisterSet();
               if (registerSetNotAllocated(fp.getOrdinal())) {
                    restoreRegisterSet();
               }
               ip.setOrdinal(getReturnInstructionPointer().getOrdinal());
            };
            switch (pfp.getReturnType()) {
                case PreviousFramePointer::ReturnStatusField::Local:
                    getFPAndIP();
                    break;
                case PreviousFramePointer::ReturnStatusField::Fault:
                    tempa = load(fp.getOrdinal() - 16);
                    tempb = load(fp.getOrdinal() - 12);
                    getFPAndIP();
                    ac.setRawValue(tempb);
                    if (pc.inSupervisorMode()) {
                        pc.setRawValue(tempa);
                    }
                    break;
                case PreviousFramePointer::ReturnStatusField::Supervisor_TraceDisabled:
                    if (!pc.inSupervisorMode()) {
                        getFPAndIP();
                    } else {
                        pc.setTraceEnableBit(false);
                        pc.enterUserMode();
                        getFPAndIP();
                    }
                    break;
                case PreviousFramePointer::ReturnStatusField::Supervisor_TraceEnabled:
                    if (!pc.inSupervisorMode()) {
                        getFPAndIP();
                    } else {
                       pc.setTraceEnableBit(true) ;
                       pc.enterUserMode();
                       getFPAndIP();
                    }
                    break;
                case PreviousFramePointer::ReturnStatusField::Reserved0:
                    // unpredictable behavior
                    break;
                case PreviousFramePointer::ReturnStatusField::Reserved1:
                    // unpredictable behavior
                    break;
                case PreviousFramePointer::ReturnStatusField::Reserved2:
                    // unpredictable behavior
                    break;
                case PreviousFramePointer::ReturnStatusField::Interrupt:
                    tempa = load(fp.getOrdinal() - 16);
                    tempb = load(fp.getOrdinal() - 12);
                    getFPAndIP();
                    ac.setRawValue(tempb);
                    if (pc.inSupervisorMode()) {
                        pc.setRawValue(tempa);
                    }
                    checkPendingInterrupts();
                    break;
            }
            doNotAdvanceIp();
        }
    }
    void
    Core::callx(Ordinal targ) {
        AnInstruction;
        auto& sp = getStackPointer();
        auto& rip = getReturnInstructionPointer();
        auto& fp = getFramePointer();
        auto pfp = getPFP();
        // the and operation clears out the least significant N bits of this new address
        // make a new stack frame
        auto tmp = (sp.getOrdinal() + computeAlignmentBoundaryConstant()) &
                   (~computeAlignmentBoundaryConstant());
        rip.setOrdinal(ip.getOrdinal() + ipIncrement_);
        saveRegisterSet();
        allocateNewLocalRegisterSet();
        ip.setOrdinal(targ);
        pfp.setRawValue(fp.getOrdinal());
        fp.setOrdinal(tmp);
        sp.setOrdinal(tmp + 64);
        doNotAdvanceIp();
    }
    void
    Core::call(Displacement22 targ) {
        AnInstruction;
        auto& sp = getStackPointer();
        auto& rip = getReturnInstructionPointer();
        auto& fp = getFramePointer();
        auto pfp = getPFP();
        auto newAddress = targ.getValue();
        // the and operation clears out the least significant N bits of this new address
        // make a new stack frame
        auto tmp = (sp.getOrdinal() + computeAlignmentBoundaryConstant()) &
                   (~computeAlignmentBoundaryConstant());
        rip.setOrdinal(ip.getOrdinal() + ipIncrement_);
        saveRegisterSet();
        allocateNewLocalRegisterSet();
        auto addr = ip.getInteger();
        ip.setInteger(addr + newAddress);
        pfp.setRawValue(fp.getOrdinal());
        fp.setOrdinal(tmp);
        sp.setOrdinal(tmp + 64);
        doNotAdvanceIp();
    }

    void
    Core::bbc(RegLit bitpos, RegisterIndex src, ShortInteger targ) {
        AnInstruction;
        auto bpos = extractValue(bitpos, TreatAsOrdinal{});
        auto theSrc = getRegisterValue(src, TreatAsOrdinal{});
        auto theMask = computeSingleBitShiftMask(bpos);
        constexpr Ordinal startingConditionCode = 0b010;
        constexpr Ordinal onConditionMet = 0b000;
        constexpr Ordinal compareAgainst = 0;
        ac.setConditionCode(startingConditionCode);
        if ((theSrc & theMask) == compareAgainst) {
            ac.setConditionCode(onConditionMet);
            ip.setInteger(ip.getInteger() + targ);
            doNotAdvanceIp();
        }
    }
    void
    Core::bbs(RegLit bitpos, RegisterIndex src, ShortInteger targ) {
        AnInstruction;
        auto bpos = extractValue(bitpos, TreatAsOrdinal{});
        auto theSrc = getRegisterValue(src, TreatAsOrdinal{});
        auto theMask = computeSingleBitShiftMask(bpos);
        constexpr Ordinal startingConditionCode = 0b000;
        constexpr Ordinal onConditionMet = 0b010;
        constexpr Ordinal compareAgainst = 1;
        ac.setConditionCode(startingConditionCode);
        if ((theSrc & theMask) == compareAgainst) {
            ac.setConditionCode(onConditionMet);
            ip.setInteger(ip.getInteger() + targ);
            doNotAdvanceIp();
        }
    }
    void
    Core::concmpo(RegLit src1, RegLit src2) {
        AnInstruction;
        conditionalCompareBase<TreatAsOrdinal>(src1, src2);
    }
    void
    Core::concmpi(RegLit src1, RegLit src2) {
        AnInstruction;
        conditionalCompareBase<TreatAsInteger>(src1, src2);
    }
    void
    Core::cmpinco(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        compareAndIncrementBase<TreatAsOrdinal>(src1, src2, dest);
    }
    void
    Core::cmpinci(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        // overflow detection suppressed
        compareAndIncrementBase<TreatAsInteger>(src1, src2, dest);
    }
    void
    Core::cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        compareAndDecrementBase<TreatAsOrdinal>(src1, src2, dest);
    }
    void
    Core::cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        // overflow detection suppressed
        compareAndDecrementBase<TreatAsInteger>(src1, src2, dest);
    }

    void
    Core::setbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        setRegister(dest, src | (1 << (bitpos & 0b11111)), TreatAsOrdinal{});
    }
    void
    Core::clrbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        auto bitposModified = ~(computeSingleBitShiftMask(bitpos));
        setRegister(dest, src & bitposModified, TreatAsOrdinal {});
    }
    void
    Core::notbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        setRegister(dest,src ^ (1 << (bitpos & 0b11111)), TreatAsOrdinal {});
    }

    void
    Core::alterbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        if ((ac.getConditionCode() & 0b010) == 0) {
            setRegister(dest, src & (~(1 << (bitpos & 0b11111))), TreatAsOrdinal{});
        } else {
            setRegister(dest, src | (1 << (bitpos & 0b11111)), TreatAsOrdinal{});
        }
    }
    void
    Core::chkbit(RegLit src1, RegLit src2) {
        AnInstruction;
        ac.setConditionCode(
                ((extractValue(src2, TreatAsOrdinal{}) & computeSingleBitShiftMask(extractValue(src1, TreatAsOrdinal{}))) == 0) ? 0b000
                                                                                                                                : 0b010);
    }
    void
    Core::spanbit(RegLit src1, RegisterIndex dest) {
        AnInstruction;
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
        setRegister(dest, result, TreatAsOrdinal{});
    }
    void
    Core::scanbit(RegLit src, RegisterIndex dest) {
        AnInstruction;
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
        setRegister(dest, result, TreatAsOrdinal{});
    }
    void
    Core::extract(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        // taken from the i960Hx manual
        setRegister(dest,
                (extractValue(dest, TreatAsOrdinal{}) >> std::min(extractValue(src1, TreatAsOrdinal{}), static_cast<Ordinal>(32))) &
                (~(0xFFFF'FFFF << extractValue(src2, TreatAsOrdinal{}))), TreatAsOrdinal {});
    }
    void
    Core::modify(RegLit mask, RegLit src, RegisterIndex srcDest) {
        AnInstruction;
        auto &sd = getRegister(srcDest);
        auto theMask = extractValue(mask, TreatAsOrdinal{});
        sd.setOrdinal((extractValue(src, TreatAsOrdinal{}) & theMask) | (sd.getOrdinal() & (~theMask)));
    }

    void
    Core::scanbyte(RegLit src1, RegLit src2) {
        AnInstruction;
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
    bool
    Core::getCarryFlag() const noexcept {
        return ac.carryFlagSet();
    }
    void
    Core::setCarryFlag(bool value) noexcept {
        ac.setCarryFlag(value);
    }
    void
    Core::b(Displacement22 targ) {
        AnInstruction;
        // force subtract 4 due to how I do things
        auto total = (ip.getInteger() + targ.getValue()) & (~0b11);
        ip.setInteger(total);
        doNotAdvanceIp();
    }
    void
    Core::bal(Displacement22 targ) {
        AnInstruction;
        globals[14].setOrdinal(ip.getOrdinal() + 4);
        // make sure that the code is consistent
        b(targ);
        doNotAdvanceIp();
    }
    void
    Core::bx(Ordinal targ) {
        AnInstruction;
        ip.setOrdinal(targ);
        doNotAdvanceIp();
    }
    void
    Core::balx(Ordinal targ, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, ip.getOrdinal() + ipIncrement_, TreatAsOrdinal {});
        ip.setOrdinal(targ);
        doNotAdvanceIp();
    }
    void
    Core::addc(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
        auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
        auto c = getCarryFlag() ? 1 : 0;
        auto result = s2 + s1 + c;
        auto upperHalf = static_cast<Ordinal>(result >> 32);
        setCarryFlag(upperHalf != 0);
        setRegister(dest, static_cast<Ordinal>(result), TreatAsOrdinal{});
        /// @todo check for integer overflow condition
    }
    void
    Core::addi(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        addGeneric(src1, src2, dest, TreatAsInteger{});
    }

    void
    Core::addo(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        addGeneric(src1, src2, dest, TreatAsOrdinal{});
        /// @todo implement fault detection
    }
    void
    Core::subi(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        subGeneric(src1, src2, dest, TreatAsInteger{});
        /// @todo implement fault detection
    }
    void
    Core::subo(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        subGeneric(src1, src2, dest, TreatAsOrdinal{});
        /// @todo implement fault detection
    }
    void
    Core::subc(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
        auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
        auto c = getCarryFlag() ? 1 : 0;
        auto result = s2 - s1 + c;
        auto upperHalf = static_cast<Ordinal>(result >> 32);
        setCarryFlag(upperHalf != 0);
        /// @todo do integer overflow subtraction check
        setRegister(dest, static_cast<Ordinal>(result), TreatAsOrdinal {});
    }
    void
    Core::muli(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        mulGeneric(src1, src2, dest, TreatAsInteger{});
        /// @todo implement fault detection
    }
    void
    Core::mulo(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        mulGeneric(src1, src2, dest, TreatAsOrdinal{});
        /// @todo implement fault detection
    }
    void
    Core::divi(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        divGeneric(src1, src2, dest, TreatAsInteger{});
        /// @todo implement fault detection
    }
    void
    Core::divo(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        divGeneric(src1, src2, dest, TreatAsOrdinal{});
        /// @todo implement fault detection
    }
    void
    Core::remi(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        remGeneric(src1, src2, dest, TreatAsInteger{});
    }
    void
    Core::remo(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        remGeneric(src1, src2, dest, TreatAsOrdinal{});
    }
    void
    Core::modi(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        // taken from the manual
        auto denominator = extractValue(src1, TreatAsInteger{});
        auto numerator = extractValue(src2, TreatAsInteger{});
        if (denominator == 0) {
            // @todo raise Arithmetic Zero Divide fault
            raiseFault();
            return;
        }
        auto theDestValue = numerator - ((numerator / denominator) * denominator);
        auto &dReg = getRegister(dest);
        dReg.setInteger(theDestValue);
        if (((numerator * denominator) < 0) && (theDestValue != 0)) {
            dReg.setInteger(theDestValue + denominator);
        }
    }
    void
    Core::shlo(RegLit len, RegLit src, RegisterIndex dest) {
        AnInstruction;
        auto theLength = extractValue(len, TreatAsOrdinal{});
        setRegister(dest, theLength < 32 ? (extractValue(src, TreatAsOrdinal {})<< theLength) : 0, TreatAsOrdinal{});
    }
    void
    Core::shro(RegLit len, RegLit src, RegisterIndex dest) {
        AnInstruction;
        auto theLength = extractValue(len, TreatAsOrdinal{});
        setRegister(dest, theLength < 32 ? (extractValue(src, TreatAsOrdinal {})>> theLength) : 0, TreatAsOrdinal{});
    }
    void
    Core::shli(RegLit len, RegLit src, RegisterIndex dest) {
        auto theLength = extractValue(len, TreatAsInteger{});
        auto theSrc = extractValue(src, TreatAsInteger{});
        setRegister(dest, theSrc << theLength, TreatAsInteger{});
    }
    /// @todo correctly implement shri and shrdi
    void
    Core::shri(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto src = extractValue(src2, TreatAsInteger{});
        auto len = abs(extractValue(src1, TreatAsInteger{}));
        if (len > 32) {
            len = 32;
        }
        setRegister(dest, src >> len, TreatAsInteger{});
    }

    void
    Core::shrdi(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto src = extractValue(src2, TreatAsInteger{});
        auto len = abs(extractValue(src1, TreatAsInteger{}));
        auto result = src >> len;
        if (src < 0 && result < 0) {
            ++result;
        }
        setRegister(dest, result, TreatAsInteger{});
    }
    void
    Core::rotate(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto len = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        setRegister(dest, rotateOperation(src, len), TreatAsOrdinal{});
    }
    void
    Core::logicalAnd(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest,
                    extractValue(src2, TreatAsOrdinal{}) & extractValue(src1, TreatAsOrdinal{}),
                    TreatAsOrdinal {});
    }
    void
    Core::andnot(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest,
                    (extractValue(src2, TreatAsOrdinal{})) & (~extractValue(src1, TreatAsOrdinal{})),
                    TreatAsOrdinal {});
    }
    void
    Core::logicalNand(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, (~extractValue(src2, TreatAsOrdinal{})) | (~extractValue(src1, TreatAsOrdinal{})), TreatAsOrdinal {});
    }

    void
    Core::logicalNor(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        auto result = (~extractValue(src2, TreatAsOrdinal{})) & (~extractValue(src1, TreatAsOrdinal{}));
        setRegister(dest, result, TreatAsOrdinal{});
    }

    void
    Core::logicalNot(RegLit src, RegisterIndex dest) {
        AnInstruction;
        auto result = ~extractValue(src, TreatAsOrdinal{});
        setRegister(dest, result, TreatAsOrdinal{});
    }
    void
    Core::notand(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::notor(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::logicalOr(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::ornot(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | (~extractValue(src1, TreatAsOrdinal{})));
    }
    void
    Core::logicalXor(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::logicalXnor(RegLit src1, RegLit src2, RegisterIndex dest) {
        AnInstruction;
        getRegister(dest).setOrdinal(~(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{})));
    }
    void
    Core::lda(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, mem, TreatAsOrdinal{});
    }
    void
    Core::ld(Ordinal address, RegisterIndex dest) {
        AnInstruction;
        auto result = load(address);
        setRegister(dest, result, TreatAsOrdinal{});
    }
    void
    Core::ldob(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, loadByteOrdinal(mem), TreatAsByteOrdinal{});
    }

    void
    Core::ldos(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, loadShortOrdinal(mem), TreatAsShortOrdinal{});
    }

    void
    Core::ldib(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, loadByteInteger(mem), TreatAsByteInteger{});
    }

    void
    Core::ldis(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, loadShortInteger(mem), TreatAsShortInteger{});
    }

    void
    Core::st(RegisterIndex src, Ordinal dest) {
        AnInstruction;
        store(dest, getRegister(src).getOrdinal());
    }

    void
    Core::stob(RegisterIndex src, Ordinal dest) {
        AnInstruction;
        storeByte(dest, getRegister(src).getByteOrdinal());
    }

    void
    Core::stib(RegisterIndex src, Ordinal dest) {
        AnInstruction;
        storeByteInteger(dest, getRegister(src).getByteInteger());
    }

    void
    Core::stis(RegisterIndex src, Ordinal dest) {
        AnInstruction;
        storeShortInteger(dest, getRegister(src).getShortInteger());
    }

    void
    Core::stos(RegisterIndex src, Ordinal dest) {
        AnInstruction;
        storeShort(dest, getRegister(src).getShortOrdinal());
    }

    void
    Core::stl(RegisterIndex src, Ordinal address) {
        AnInstruction;
        if (!divisibleByTwo(src)) {
            /// @todo raise a operation.invalid_operand fault
            raiseFault();
        } else {
            store(address, getRegisterValue(src, TreatAsOrdinal{}));
            store(address + 4, getRegisterValue(nextRegisterIndex(src), TreatAsOrdinal{}));
            if ((address & 0b111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED fault
                raiseFault();
            }
        }
    }
    void
    Core::stt(RegisterIndex src, Ordinal address) {
        AnInstruction;
        if (!divisibleByFour(src)) {
            /// @todo raise a operation.invalid_operand fault
            raiseFault();
        } else {
            store(address, getRegisterValue(src, TreatAsOrdinal{}));
            store(address + 4, getRegisterValue(nextRegisterIndex(src), TreatAsOrdinal{}));
            store(address + 8, getRegisterValue(nextRegisterIndex(nextRegisterIndex(src)), TreatAsOrdinal{}));
            if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }

    void
    Core::stq(RegisterIndex src, Ordinal address) {
        AnInstruction;
        if (!divisibleByFour(src)) {
            raiseFault();
            /// @todo raise a operation.invalid_operand fault
        } else {
            store(address, getRegisterValue(src, TreatAsOrdinal{}));
            store(address + 4, getRegisterValue(nextRegisterIndex(src), TreatAsOrdinal{}));
            store(address + 8, getRegisterValue(nextRegisterIndex(nextRegisterIndex(src)), TreatAsOrdinal{}));
            store(address + 12, getRegisterValue(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(src))), TreatAsOrdinal{}));
            if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                raiseFault();
                /// @todo generate an OPERATION.UNALIGNED_FAULT
            }
        }
    }

    void
    Core::ldl(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        if (!divisibleByTwo(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            setRegister(dest, -1, TreatAsOrdinal{});
            raiseFault();
        } else {
            setRegister(dest, load(mem), TreatAsOrdinal{});
            setRegister(nextRegisterIndex(dest), load(mem + 4), TreatAsOrdinal{});
            if ((mem & 0b111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }
    void
    Core::ldt(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        if (!divisibleByFour(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            setRegister(dest, -1, TreatAsOrdinal{});
            raiseFault();
        } else {
            setRegister(dest, load(mem), TreatAsOrdinal{});
            setRegister(nextRegisterIndex(dest), load(mem + 4), TreatAsOrdinal{});
            setRegister(nextRegisterIndex(nextRegisterIndex(dest)), load(mem + 8), TreatAsOrdinal{});
            if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }
    void
    Core::ldq(Ordinal mem, RegisterIndex dest) {
        AnInstruction;
        if (!divisibleByFour(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            getRegister(dest).setOrdinal(-1);
            raiseFault();
        } else {
            setRegister(dest, load(mem), TreatAsOrdinal{});
            setRegister(nextRegisterIndex(dest), load(mem + 4), TreatAsOrdinal{});
            setRegister(nextRegisterIndex(nextRegisterIndex(dest)), load(mem + 8), TreatAsOrdinal{});
            setRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest))), load(mem + 12), TreatAsOrdinal{});
            if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }

    void
    Core::mov(RegLit src, RegisterIndex dest) {
        AnInstruction;
        setRegister(dest, extractValue(src, TreatAsOrdinal{}), TreatAsOrdinal{});
    }
    void
    Core::movl(RegLit src, RegisterIndex dest) {
        AnInstruction;
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
    Core::movt(RegLit src, RegisterIndex dest) {
        AnInstruction;
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
    Core::movq(RegLit src, RegisterIndex dest) {
        AnInstruction;
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
            getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(
                    extractValue(nextValue(nextValue(nextValue(src))), TreatAsOrdinal{}));
        }
    }
    void
    Core::allocateNewLocalRegisterSet() {
        /// @todo implement at some point
    }
    Ordinal
    Core::bootChecksum() noexcept {
        /// @todo compute checksum properly
        return 0;
    }

    void
    Core::post() {
        /// load the IBR
        /// @todo handle the PMCON setup as described by the i960Hx manual
        if (bootChecksum() != 0) {
            busTestFailed();
        }
        static constexpr auto shouldProcessPCRB = false;
        if constexpr (shouldProcessPCRB) {
            prcbBase_ = load(ibrBase_ + 0x14);
            ctrlTableBase_ = load(prcbBase_ + 0x4);
            processPRCB();
        }
        ip.setOrdinal(load(ibrBase_ + 0x10));
        /// @todo set the device id correctly
        getRegister(g0).setOrdinal(0xFDEDABCD);
        /// and done...
    }
    void
    Core::processPRCB() noexcept {
        /// @todo implement based off of the i960Hx manual
        /// setup the internal peripherals
        auto pcrbMMR = prcbBase_;
        faultTableBase_ = load(pcrbMMR);
        ctrlTableBase_ = load(pcrbMMR + 0x4);
        ac.setRawValue(load(pcrbMMR + 0x8));
        auto faultConfig = load(pcrbMMR + 0xc);

        if ((faultConfig >> 30) & 1) {
            _unalignedFaultEnabled = false;
        } else {
            _unalignedFaultEnabled = true;
        }
        // load interrupt table and cache nmi vector entry
        // resetBlockNMI
        interruptTableBase_= load(pcrbMMR + 0x10);
        nmiVector_ = load(interruptTableBase_ + (248 * 4) + 4);
        // process system procedure table
        // initialize isp, fp, sp, and pfp
        // initialize instruction cache
        // configure local register cache
        // load control table
    }
    PreviousFramePointer Core::getPFP() noexcept {
        return PreviousFramePointer{getRegister(PFP)};
    }
    Register& Core::getStackPointer() noexcept {
        return getRegister(SP);
    }
    Register& Core::getFramePointer() noexcept {
        return getRegister(FP);
    }
    Register& Core::getReturnInstructionPointer() noexcept {
        return getRegister(RIP);
    }
    void Core::freeCurrentRegisterSet() noexcept {
        /// @todo implement when support for register sets is provided
    }


    void
    Core::checkPendingInterrupts() noexcept {
        /// @todo implement support for this later on
    }
    Ordinal
    Core::getSystemProcedureTableBase() noexcept {
        /// @todo implement
        return 0;
    }
    Ordinal
    Core::getSupervisorStackPointer() noexcept {
        return load(getSystemProcedureTableBase() + 0xC);
    }
    ByteOrdinal
    Core::loadByteOrdinal(Address address) noexcept {
        return static_cast<ByteOrdinal>(load(address));
    }
    ByteInteger
    Core::loadByteInteger(Address address) noexcept {
        return static_cast<ByteInteger>(load(address));
    }
    ShortOrdinal
    Core::loadShortOrdinal(Address address) noexcept {
        return static_cast<ShortOrdinal>(load(address));
    }
    ShortInteger
    Core::loadShortInteger(Address address) noexcept {
        return static_cast<ShortInteger>(load(address));
    }
    void
    Core::storeByteInteger(Address address, ByteInteger value) noexcept {
        union {
            ByteInteger value;
            ByteOrdinal ordValue;
        } k;
        k.value = value;
        storeByte(address, k.ordValue);
    }
    void
    Core::storeShortInteger(Address address, ShortInteger value) noexcept {
        union {
            ShortInteger value;
            ShortOrdinal ordValue;
        } k;
        k.value = value;
        storeShort(address, k.ordValue);
    }
    void Core::cmpo(RegLit src1, RegLit src2) {
        AnInstruction;
        compareBase<TreatAsOrdinal>(src1, src2);
    }
    void
    Core::cmpi(RegLit src1, RegLit src2) {
        AnInstruction;
        compareBase<TreatAsInteger>(src1, src2);
    }
    void
    Core::doNotAdvanceIp() noexcept {
        ipIncrement_ = 0;
    }
    void
    Core::instructionIsDoubleWide() noexcept {
        ipIncrement_ = 8;
    }
}
#undef AnInstruction

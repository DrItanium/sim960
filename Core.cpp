//
// Created by jwscoggins on 12/5/20.
//
#include "ArithmeticControls.h"
#include "Core.h"

namespace i960 {
    constexpr Ordinal largestOrdinal = 0xFFFF'FFFF;
    constexpr RegisterIndex PFP = static_cast<RegisterIndex>(0b00000);
    constexpr RegisterIndex SP = static_cast<RegisterIndex>(0b00001);
    constexpr RegisterIndex RIP = static_cast<RegisterIndex>(0b00010);
    constexpr RegisterIndex FP = static_cast<RegisterIndex>(0b11111);
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
    /**
     * @brief Retrieve the word at the ip address
     * @param advance
     * @return
     */
    Ordinal
    Core::getWordAtIP(bool advance) noexcept {
        auto ipLoc = ip.getOrdinal();
        if (advance) {
            nextInstruction();
        }
        return loadOrdinal(ipLoc);
    }
    Ordinal
    Core::extractValue(RegLit value, TreatAsOrdinal) const noexcept {
        return std::visit([this](auto &&value) -> Ordinal {
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
    Integer
    Core::extractValue(RegLit value, TreatAsInteger) const noexcept {
        return std::visit([this](auto &&value) -> Integer {
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
        // a total copy and paste hack job but it will work
        if (std::holds_alternative<Literal>(src2)) {
            // this is a little different than normal
            auto denominator = extractValue(src1, TreatAsOrdinal{});
            if (!divisibleByTwo(dest)) {
                getRegister(dest).setOrdinal(-1);
                getRegister(nextRegisterIndex(dest)).setOrdinal(-1);
                raiseFault(); // OPERATION.INVALID_OPERAND
            } else if (denominator == 0) {
                getRegister(dest).setOrdinal(-1);
                getRegister(nextRegisterIndex(dest)).setOrdinal(-1);
                raiseFault(); // ARITHMETIC.DIVIDE_ZERO
            } else {
                auto numerator = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
                auto quotient = numerator / denominator;
                auto remainder = numerator - (numerator / denominator) * denominator;
                getRegister(nextRegisterIndex(dest)).setOrdinal(quotient);
                getRegister(dest).setOrdinal(remainder);
            }
        } else {
            // okay we are holding onto a RegisterIndex in src2
            auto s2ri = std::get<RegisterIndex>(src2);
            auto denominator = extractValue(src1, TreatAsOrdinal{});
            if (!divisibleByTwo(s2ri) || !divisibleByTwo(dest)) {
                getRegister(dest).setOrdinal(-1);
                getRegister(nextRegisterIndex(dest)).setOrdinal(-1);
                raiseFault(); // OPERATION.INVALID_OPERAND
            } else if (denominator == 0) {
                getRegister(dest).setOrdinal(-1);
                getRegister(nextRegisterIndex(dest)).setOrdinal(-1);
                raiseFault(); // ARITHMETIC.DIVIDE_ZERO
            } else {
                LongRegister tmp(getRegister(s2ri),
                                 getRegister(nextRegisterIndex(s2ri)));
                auto numerator = tmp.getOrdinal();
                auto quotient = numerator / denominator;
                auto remainder = numerator - (numerator / denominator) * denominator;
                getRegister(nextRegisterIndex(dest)).setOrdinal(quotient);
                getRegister(dest).setOrdinal(remainder);
            }
        }
    }

    void
    Core::emul(RegLit src1, RegLit src2, RegisterIndex dest) {
        if (!divisibleByTwo(dest)) {
            getRegister(dest).setOrdinal(-1);
            getRegister(nextRegisterIndex(dest)).setOrdinal(-1);
            raiseFault(); // OPERATION.INVALID_OPERAND
        } else {
            auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
            auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
            auto result = s1 * s2;
            getRegister(dest).setOrdinal(result);
            getRegister(nextRegisterIndex(dest)).setOrdinal(static_cast<Ordinal>(result >> 32));
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
    Core::nextInstruction() {
        ip.setOrdinal(ip.getOrdinal() + 4);
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
            case 0x310: compareAndBranch<ConditionCodeKind::GreaterThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x320: compareAndBranch<ConditionCodeKind::EqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x330: compareAndBranch<ConditionCodeKind::GreaterThanOrEqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x340: compareAndBranch<ConditionCodeKind::LessThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x350: compareAndBranch<ConditionCodeKind::NotEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x360: compareAndBranch<ConditionCodeKind::LessThanOrEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x370: bbs(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x380: compareAndBranch<ConditionCodeKind::Unordered>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x390: compareAndBranch<ConditionCodeKind::GreaterThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3A0: compareAndBranch<ConditionCodeKind::EqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3B0: compareAndBranch<ConditionCodeKind::GreaterThanOrEqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3C0: compareAndBranch<ConditionCodeKind::LessThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3D0: compareAndBranch<ConditionCodeKind::NotEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3E0: compareAndBranch<ConditionCodeKind::LessThanOrEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3F0: compareAndBranch<ConditionCodeKind::Ordered>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
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
        auto lower = getWordAtIP(true);
        auto upper = getWordAtIP(false); // do not automatically advance this time
        cycle(lower, upper);
    }
    void
    Core::cycle(Ordinal lower, Ordinal upper) {
        executeInstruction(decode(lower, upper));
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
        auto ival = toInteger(index);
        if (auto offset = ival & 0b1111, maskedValue = ival & 0b10000; maskedValue != 0) {
            return locals[offset];
        } else {
            return globals[offset];
        }
    }
    const Register &
    Core::getRegister(RegisterIndex index) const noexcept {
        auto ival = toInteger(index);
        if (auto offset = ival & 0b1111, maskedValue = ival & 0b10000; maskedValue != 0) {
            return locals[offset];
        } else {
            return globals[offset];
        }
    }
    void
    Core::saveRegisterSet() noexcept {
        // okay, we have to save all of the registers to the stack or the on board
        // register cache (however, I'm not implementing that yet)
    }
    void
    Core::restoreRegisterSet() noexcept {
        // restore the local register frame, generally done when you return from a
        // previous function
    }

    void
    Core::dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
        const auto &s1 = getRegister(src1);
        const auto &s2 = getRegister(src2);
        // transfer bits over
        auto outcome = (s2.getOrdinal() - s1.getOrdinal() - 1 + (getCarryFlag() ? 1 : 0)) & 0xF;
        setCarryFlag(outcome != 0);
        getRegister(dest).setOrdinal((s2.getOrdinal() & (~0xF)) | (outcome & 0xF));
    }
    void
    Core::dmovt(RegisterIndex src1, RegisterIndex dest) {
        auto srcValue = extractValue(src1, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(srcValue);
        auto lowest8 = static_cast<ByteOrdinal>(srcValue);
        ac.setConditionCode(((lowest8 >= 0b0011'0000) && (lowest8 <= 0b0011'1001)) ? 0b000 : 0b010);
    }
    void
    Core::daddc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
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
        if (pc.getTraceEnableBit()) {
            raiseFault(); // TRACE BREAKPOINT FAULT
        }
    }
    void
    Core::mark() {
        if (pc.getTraceEnableBit() && tc.getMarkTraceMode()) {
            raiseFault(); // TRACE.MARK
        }
    }
    void
    Core::syncf() {
        if (ac.getNoImpreciseFaults()) {
            return;
        }
        // do a noop
    }
    void
    Core::flushreg() {
        // noop right now
    }
    RegisterIndex
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
        auto dest = forceIntoRegisterIndex(inst.getSrc1());
        auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
        auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
        getRegister(dest).setOrdinal(tc.modify(mask, src));
    }
    void
    Core::modpc(const RegFormatInstruction &inst) {
        auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
        auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
        auto dest = forceIntoRegisterIndex(inst.getSrc1());
        auto tmp = pc.getRawValue();
        pc.setRawValue((src & mask) | (tmp & (~mask)));
        getRegister(dest).setOrdinal(tmp);
    }
    void
    Core::modac(const RegFormatInstruction &inst) {
        // in this case, mask is src/dst
        // src is src2
        // dest is src1
        auto mask = extractValue(inst.getSrcDest(), TreatAsOrdinal{});
        auto src = extractValue(inst.getSrc2(), TreatAsOrdinal{});
        auto dest = forceIntoRegisterIndex(inst.getSrc1());
        auto tmp = ac.getRawValue();
        ac.setRawValue((src & mask) | (tmp & (~mask)));
        getRegister(dest).setOrdinal(tmp);
    }
    Ordinal
    Core::getSystemProcedureEntry(Ordinal targ) noexcept {
        /// @todo implement
        return 0;
    }
    bool
    Core::registerSetAvailable() const noexcept {
        /// @todo update this when I implement proper register sets
        return false;
    }
    void
    Core::calls(RegLit targ) {
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
            /// @todo implement this psuedo-code
            // RIP = IP
            // IP[31:2] = effective_address(temp[31:2]);
            // IP[1:0] = 0;
            // if ((temp.type == local) || (pc.inSupervisorMode()) {
            // Local call or supervisor call from supervisor mode
            // tempa = (sp + (salign * 16 - 1)) & ~(salign * 16 - 1));
            // round stack pointer to next boundary
            // salign = 1 on i960 Hx processors
            // temp.rrr = 000;
            // } else {
            // tempa = SSP; // get supervisor stack pointer
            // temp.rrr = 0b010 | pc.te;
            // pc.em = supervisor;
            // pc.te = temp.te;
            // }
            // PFP = fp;
            // pfp.rrr = temp.rrr;
            // fp = tempa;
            // sp = tempa + 64;

        }
    }
    void
    Core::ret() {
        syncf();
        /// @todo continue implementing
    }
    void
    Core::callx(Ordinal targ) {

        // the and operation clears out the least significant N bits of this new address
        // make a new stack frame
        auto tmp = (getStackPointerAddress() + computeAlignmentBoundaryConstant()) &
                   (~computeAlignmentBoundaryConstant());
        setRIP(ip);
        saveRegisterSet();
        allocateNewLocalRegisterSet();
        ip.setOrdinal(targ);
        setPFP(getFramePointerAddress());
        setFramePointer(tmp);
        setStackPointer(tmp + 64);
    }
    void
    Core::call(Displacement22 targ) {
        auto newAddress = targ.getValue();
        // the and operation clears out the least significant N bits of this new address
        // make a new stack frame
        auto tmp = (getStackPointerAddress() + computeAlignmentBoundaryConstant()) &
                   (~computeAlignmentBoundaryConstant());
        setRIP(ip);
        saveRegisterSet();
        allocateNewLocalRegisterSet();
        auto addr = ip.getInteger();
        ip.setInteger(addr + newAddress);
        setPFP(getFramePointerAddress());
        setFramePointer(tmp);
        setStackPointer(tmp + 64);
    }

    void
    Core::bbc(RegLit bitpos, RegisterIndex src, ShortInteger targ) {
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
    Core::bbs(RegLit bitpos, RegisterIndex src, ShortInteger targ) {
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
    Core::concmpo(RegLit src1, RegLit src2) {
        conditionalCompareBase<TreatAsOrdinal>(src1, src2);
    }
    void
    Core::concmpi(RegLit src1, RegLit src2) {
        conditionalCompareBase<TreatAsInteger>(src1, src2);
    }
    void
    Core::cmpinco(RegLit src1, RegLit src2, RegisterIndex dest) {
        compareAndIncrementBase<TreatAsOrdinal>(src1, src2, dest);
    }
    void
    Core::cmpinci(RegLit src1, RegLit src2, RegisterIndex dest) {
        // overflow detection suppressed
        compareAndIncrementBase<TreatAsInteger>(src1, src2, dest);
    }
    void
    Core::cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest) {
        compareAndDecrementBase<TreatAsOrdinal>(src1, src2, dest);
    }
    void
    Core::cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest) {
        // overflow detection suppressed
        compareAndDecrementBase<TreatAsInteger>(src1, src2, dest);
    }

    void
    Core::setbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
    }
    void
    Core::clrbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        auto bitposModified = ~(computeSingleBitShiftMask(bitpos));
        getRegister(dest).setOrdinal(src & bitposModified);
    }
    void
    Core::notbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(src ^ (1 << (bitpos & 0b11111)));
    }

    void
    Core::alterbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        if ((ac.getConditionCode() & 0b010) == 0) {
            getRegister(dest).setOrdinal(src & (~(1 << (bitpos & 0b11111))));
        } else {
            getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
        }
    }
    void
    Core::chkbit(RegLit src1, RegLit src2) {
        ac.setConditionCode(
                ((extractValue(src2, TreatAsOrdinal{}) & computeSingleBitShiftMask(extractValue(src1, TreatAsOrdinal{}))) == 0) ? 0b000
                                                                                                                                : 0b010);
    }
    void
    Core::spanbit(RegLit src1, RegisterIndex dest) {
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
    Core::scanbit(RegLit src, RegisterIndex dest) {
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
    Core::extract(RegLit src1, RegLit src2, RegisterIndex dest) {
        // taken from the i960Hx manual
        getRegister(dest).setOrdinal(
                (extractValue(dest, TreatAsOrdinal{}) >> std::min(extractValue(src1, TreatAsOrdinal{}), static_cast<Ordinal>(32))) &
                (~(0xFFFF'FFFF << extractValue(src2, TreatAsOrdinal{}))));
    }
    void
    Core::modify(RegLit mask, RegLit src, RegisterIndex srcDest) {
        auto &sd = getRegister(srcDest);
        auto theMask = extractValue(mask, TreatAsOrdinal{});
        sd.setOrdinal((extractValue(src, TreatAsOrdinal{}) & theMask) | (sd.getOrdinal() & (~theMask)));
    }

    void
    Core::scanbyte(RegLit src1, RegLit src2) {
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
    /// @todo figure out correct signatures
    void
    Core::b(Displacement22 targ) {
        ip.setInteger(ip.getInteger() + targ.getValue());
    }
    void
    Core::bal(Displacement22 targ) {
        globals[14].setOrdinal(ip.getOrdinal() + 4);
        // make sure that the code is consistent
        b(targ);
    }
    void
    Core::bx(Ordinal targ) {
        ip.setOrdinal(targ);
    }
    void
    Core::balx(Ordinal targ, RegisterIndex dest) {
        getRegister(dest).setOrdinal(ip.getOrdinal());
        ip.setOrdinal(targ);
    }
    void
    Core::addc(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
        auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
        auto c = getCarryFlag() ? 1 : 0;
        auto result = s2 + s1 + c;
        auto upperHalf = static_cast<Ordinal>(result >> 32);
        setCarryFlag(upperHalf != 0);
        getRegister(dest).setOrdinal(static_cast<Ordinal>(result));
        /// @todo check for integer overflow condition
    }
    void
    Core::addi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{});
        auto s2 = extractValue(src2, TreatAsInteger{});
        getRegister(dest).setInteger(s2 + s1);
        /// @todo implement fault detection
    }

    void
    Core::addo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{});
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(s2 + s1);
        /// @todo implement fault detection
    }
    void
    Core::subi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{});
        auto s2 = extractValue(src2, TreatAsInteger{});
        getRegister(dest).setInteger(s2 - s1);
        /// @todo implement fault detection
    }
    void
    Core::subo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{});
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(s2 - s1);
        /// @todo implement fault detection
    }
    void
    Core::subc(RegLit src1, RegLit src2, RegisterIndex dest) {
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
    Core::muli(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{});
        auto s2 = extractValue(src2, TreatAsInteger{});
        getRegister(dest).setInteger(s2 * s1);
        /// @todo implement fault detection
    }
    void
    Core::mulo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{});
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(s2 * s1);
        /// @todo implement fault detection
    }
    void
    Core::divi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{});
        auto s2 = extractValue(src2, TreatAsInteger{});
        getRegister(dest).setInteger(s2 / s1);
        /// @todo implement fault detection
    }
    void
    Core::divo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{});
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(s2 / s1);
        /// @todo implement fault detection
    }
    void
    Core::remi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s2 = extractValue(src2, TreatAsInteger{});
        auto s1 = extractValue(src1, TreatAsInteger{});
        getRegister(dest).setInteger(((s2 / s1) * s1));
    }
    void
    Core::remo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        auto s1 = extractValue(src1, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(((s2 / s1) * s1));
    }
    void
    Core::modi(RegLit src1, RegLit src2, RegisterIndex dest) {
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
        auto theLength = extractValue(len, TreatAsOrdinal{});
        auto theSrc = extractValue(src, TreatAsOrdinal{});
        if (theLength < 32) {
            getRegister(dest).setOrdinal(theSrc << theLength);
        } else {
            getRegister(dest).setOrdinal(0);
        }
    }
    void
    Core::shro(RegLit len, RegLit src, RegisterIndex dest) {
        auto theLength = extractValue(len, TreatAsOrdinal{});
        auto theSrc = extractValue(src, TreatAsOrdinal{});
        if (theLength < 32) {
            getRegister(dest).setOrdinal(theSrc >> theLength);
        } else {
            getRegister(dest).setOrdinal(0);
        }
    }
    void
    Core::shli(RegLit len, RegLit src, RegisterIndex dest) {
        auto theLength = extractValue(len, TreatAsInteger{});
        auto theSrc = extractValue(src, TreatAsInteger{});
        getRegister(dest).setInteger(theSrc << theLength);
    }
    /// @todo correctly implement shri and shrdi
    void
    Core::shri(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto src = extractValue(src2, TreatAsInteger{});
        auto len = abs(extractValue(src1, TreatAsInteger{}));
        if (len > 32) {
            len = 32;
        }
        getRegister(dest).setInteger(src >> len);
    }

    void
    Core::shrdi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto src = extractValue(src2, TreatAsInteger{});
        auto len = abs(extractValue(src1, TreatAsInteger{}));
        auto result = src >> len;
        if (src < 0 && result < 0) {
            ++result;
        }
        getRegister(dest).setInteger(result);
    }
    void
    Core::rotate(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto len = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(rotateOperation(src, len));
    }
    void
    Core::logicalAnd(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(
                extractValue(src2, TreatAsOrdinal{}) &
                extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::andnot(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(
                (extractValue(src2, TreatAsOrdinal{})) &
                (~extractValue(src1, TreatAsOrdinal{})));
    }
    void
    Core::logicalNand(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | (~extractValue(src1, TreatAsOrdinal{})));
    }

    void
    Core::logicalNor(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & (~extractValue(src1, TreatAsOrdinal{})));
    }

    void
    Core::logicalNot(RegLit src, RegisterIndex dest) {
        getRegister(dest).setOrdinal(~extractValue(src, TreatAsOrdinal{}));
    }
    void
    Core::notand(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::notor(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::logicalOr(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::ornot(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | (~extractValue(src1, TreatAsOrdinal{})));
    }
    void
    Core::logicalXor(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{}));
    }
    void
    Core::logicalXnor(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(~(extractValue(src2, TreatAsOrdinal{}) ^ extractValue(src1, TreatAsOrdinal{})));
    }
    void
    Core::lda(Ordinal mem, RegisterIndex dest) {
        getRegister(dest).setOrdinal(mem);
    }
    void
    Core::ld(Ordinal address, RegisterIndex dest) {
        getRegister(dest).setOrdinal(loadOrdinal(address));
    }
    void
    Core::ldob(Ordinal mem, RegisterIndex dest) {
        getRegister(dest).setByteOrdinal(loadByteOrdinal(mem));
    }

    void
    Core::ldos(Ordinal mem, RegisterIndex dest) {
        getRegister(dest).setShortOrdinal(loadShortOrdinal(mem));
    }

    void
    Core::ldib(Ordinal mem, RegisterIndex dest) {
        getRegister(dest).setByteInteger(loadByteInteger(mem));
    }

    void
    Core::ldis(Ordinal mem, RegisterIndex dest) {
        getRegister(dest).setShortInteger(loadShortInteger(mem));
    }

    void
    Core::st(RegisterIndex src, Ordinal dest) {
        storeOrdinal(dest, getRegister(src).getOrdinal());
    }

    void
    Core::stob(RegisterIndex src, Ordinal dest) {
        storeByteOrdinal(dest, getRegister(src).getByteOrdinal());
    }

    void
    Core::stib(RegisterIndex src, Ordinal dest) {
        storeByteInteger(dest, getRegister(src).getByteInteger());
    }

    void
    Core::stis(RegisterIndex src, Ordinal dest) {
        storeShortInteger(dest, getRegister(src).getShortInteger());
    }

    void
    Core::stos(RegisterIndex src, Ordinal dest) {
        storeShortOrdinal(dest, getRegister(src).getShortOrdinal());
    }

    void
    Core::stl(RegisterIndex src, Ordinal address) {
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
    Core::stt(RegisterIndex src, Ordinal address) {
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
    Core::stq(RegisterIndex src, Ordinal address) {
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
    Core::ldl(Ordinal mem, RegisterIndex dest) {
        if (!divisibleByTwo(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            getRegister(dest).setOrdinal(-1);
            raiseFault();
        } else {
            getRegister(dest).setOrdinal(loadOrdinal(mem));
            getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem + 4));
            if ((mem & 0b111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }
    void
    Core::ldt(Ordinal mem, RegisterIndex dest) {
        if (!divisibleByFour(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            getRegister(dest).setOrdinal(-1);
            raiseFault();
        } else {
            getRegister(dest).setOrdinal(loadOrdinal(mem));
            getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem + 4));
            getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem + 8));
            if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }
    void
    Core::ldq(Ordinal mem, RegisterIndex dest) {
        if (!divisibleByFour(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            getRegister(dest).setOrdinal(-1);
            raiseFault();
        } else {
            getRegister(dest).setOrdinal(loadOrdinal(mem));
            getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem + 4));
            getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem + 8));
            getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(loadOrdinal(mem + 12));
            if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
                raiseFault();
            }
        }
    }

    void
    Core::mov(RegLit src, RegisterIndex dest) {
        getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
    }
    void
    Core::movl(RegLit src, RegisterIndex dest) {
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
    Ordinal
    Core::getStackPointerAddress() const noexcept {
        return getRegister(SP).getOrdinal();
    }
    void
    Core::setRIP(const Register &ip) noexcept {
        getRegister(RIP).setOrdinal(ip.getOrdinal());
    }
    Ordinal
    Core::getFramePointerAddress() const noexcept {
        return getRegister(FP).getOrdinal();
    }
    void
    Core::setPFP(Ordinal value) noexcept {
        getRegister(PFP).setOrdinal(value);
    }
    void
    Core::setFramePointer(Ordinal value) noexcept {
        getRegister(FP).setOrdinal(value);
    }
    void
    Core::setStackPointer(Ordinal value) noexcept {
        getRegister(SP).setOrdinal(value);
    }
    void
    Core::allocateNewLocalRegisterSet() {
        /// @todo implement at some point
    }

    void
    Core::post() {

    }

    std::string
    RegFormatInstruction::decodeName() const noexcept {
        if (auto result = getNameFromOpcode(getOpcode()); result) {
            return *result;
        } else {
            return "";
        }
    }
    std::string
    COBRInstruction::decodeName() const noexcept {
        if (auto result = getNameFromOpcode(getOpcode()); result) {
            return *result;
        } else {
            return "";
        }
    }
    std::string
    CTRLInstruction::decodeName() const noexcept {
        if (auto result = getNameFromOpcode(getOpcode()); result) {
            return *result;
        } else {
            return "";
        }
    }
    std::string
    MEMFormatInstruction::decodeName() const noexcept {
        if (auto result = getNameFromOpcode(getOpcode()); result) {
            return *result;
        } else {
            return "";
        }
    }
}
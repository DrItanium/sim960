//
// Created by jwscoggins on 12/5/20.
//
#include "ArithmeticControls.h"
#include "Core.h"

namespace i960 {
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
    Core::execute(const RegFormatInstruction& inst) noexcept {
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
    Core::execute(const MEMFormatInstruction &inst) noexcept {
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
    Core::execute(const COBRInstruction &inst) noexcept {
        switch (inst.getOpcode()) {
            case 0x200: testno(inst.getSrc1()); break;
            case 0x210: testg(inst.getSrc1()); break;
            case 0x220: teste(inst.getSrc1()); break;
            case 0x230: testge(inst.getSrc1()); break;
            case 0x240: testl(inst.getSrc1()); break;
            case 0x250: testne(inst.getSrc1()); break;
            case 0x260: testle(inst.getSrc1()); break;
            case 0x270: testo(inst.getSrc1()); break;
            case 0x300: bbc(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x310: compareAndBranch<ConditionCodeKind::GreaterThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal { }); break;
            case 0x320: compareAndBranch<ConditionCodeKind::EqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal { }); break;
            case 0x330: compareAndBranch<ConditionCodeKind::GreaterThanOrEqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal{}); break;
            case 0x340: compareAndBranch<ConditionCodeKind::LessThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal { }); break;
            case 0x350: compareAndBranch<ConditionCodeKind::NotEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal { }); break;
            case 0x360: compareAndBranch<ConditionCodeKind::LessThanOrEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsOrdinal { }); break;
            case 0x370: bbs(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x380: compareAndBranch<ConditionCodeKind::Unordered>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x390: compareAndBranch<ConditionCodeKind::GreaterThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3A0: compareAndBranch<ConditionCodeKind::EqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3B0: compareAndBranch<ConditionCodeKind::GreaterThanOrEqualTo>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3C0: compareAndBranch<ConditionCodeKind::LessThan>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3D0: compareAndBranch<ConditionCodeKind::NotEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3E0: compareAndBranch<ConditionCodeKind::LessThanOrEqual>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            case 0x3F0: compareAndBranch<ConditionCodeKind::Ordered>(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement(), TreatAsInteger{}); break;
            default:
                raiseFault();
                break;
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
            default:
                raiseFault();
                break;
        }
    }
    void
    Core::cycle() {
        executeInstruction(decodeInstruction(fetchInstruction()));
    }
    Core::DecodedInstruction
    Core::decodeInstruction(Ordinal currentInstruction) {
        return decode(currentInstruction);
    }
    void
    Core::executeInstruction(const DecodedInstruction& inst) {
        std::visit([this](auto&& theInst) { execute(theInst); }, inst);
    }
    Ordinal
    Core::fetchInstruction() {
        return getWordAtIP(true);
    }
    void
    Core::raiseFault() {
        /// @todo implement
    }
    Register&
    Core::getRegister(RegisterIndex index) noexcept {
        auto ival = toInteger(index);
        if (auto offset = ival & 0b1111, maskedValue = ival & 0b10000; maskedValue != 0) {
            return locals[offset];
        } else {
            return globals[offset];
        }
    }
    const Register&
    Core::getRegister(RegisterIndex index) const noexcept {
        auto ival = toInteger(index);
        if (auto offset = ival & 0b1111, maskedValue = ival & 0b10000; maskedValue != 0) {
            return locals[offset];
        } else {
            return globals[offset];
        }
    }
    void
    Core::saveLocals() noexcept {
        // okay, we have to save all of the registers to the stack or the on board
        // register cache (however, I'm not implementing that yet)
    }
    void
    Core::restoreLocals() noexcept {
        // restore the local register frame, generally done when you return from a
        // previous function
    }

    void
    Core::dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
        /// @todo implement... such a baffling design...BCD...
    }
    void
    Core::dmovt(RegisterIndex src1, RegisterIndex dest) {
        auto srcValue = extractValue(src1, TreatAsOrdinal { });
        getRegister(dest).setOrdinal(srcValue);
        auto lowest8 = static_cast<ByteOrdinal>(srcValue);
        ac.setConditionCode(((lowest8 >= 0b0011'0000) && (lowest8 <= 0b0011'1001)) ? 0b000 : 0b010);
    }
    void
    Core::daddc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {
        const auto& s1 = getRegister(src1);
        const auto& s2 = getRegister(src2);
        auto& dst = getRegister(dest);
        // transfer bits over
        dst.setOrdinal(s2.getOrdinal());
        auto outcome = (s2.getByteOrdinal() & 0xF) + (s1.getByteOrdinal() & 0xF) + (getCarryFlag() ? 1 : 0);
        setCarryFlag((outcome & 0xF0) != 0);
        dst.setByteOrdinal((s2.getByteOrdinal() & 0xF0) | (outcome & 0x0F));
    }
    void
    Core::fmark() {
        /// @todo implement
    }
    void
    Core::mark() {
        /// @todo implement
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
    void
    Core::modtc(const RegFormatInstruction& inst) {
        /// @todo implement
    }
    void
    Core::modpc(const RegFormatInstruction& inst) {
        /// @todo implement
    }
    void
    Core::modac(const RegFormatInstruction &inst) {
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
    Core::calls(RegLit targ) {
        /// @todo implement
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
        saveLocals();
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
        saveLocals();
        allocateNewLocalRegisterSet();
        auto addr = ip.getInteger();
        ip.setInteger(addr + newAddress);
        setPFP(getFramePointerAddress());
        setFramePointer(tmp);
        setStackPointer(tmp + 64);
    }
    void
    Core::testo(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsOrdered() ? 1 : 0);
    }
    void
    Core::testno(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.getConditionCode() == 0 ? 1 : 0);
    }
    void
    Core::teste(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsEqualTo() ? 1 : 0);
    }
    void
    Core::testne(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsNotEqual() ? 1 : 0);
    }
    void
    Core::testl(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsLessThan() ? 1 : 0);
    }
    void
    Core::testle(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsLessThanOrEqual() ? 1 : 0);
    }
    void
    Core::testg(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsGreaterThan() ? 1 : 0);
    }
    void
    Core::testge(RegisterIndex dest) {
        getRegister(dest).setOrdinal(ac.conditionIsGreaterThanOrEqualTo() ? 1 : 0);
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
    Core::cmpo(RegLit src1, RegLit src2) {
        compareBase<TreatAsOrdinal>(src1, src2);
    }
    void
    Core::cmpi(RegLit src1, RegLit src2) {
        compareBase<TreatAsInteger>(src1, src2);
    }
    void
    Core::concmpo(RegLit src1, RegLit src2) {
        // don't care what the least significant two bits are of the cond code so just mask them out
        if ((ac.getConditionCode() & 0b100) == 0) {
            auto s1 = extractValue(src1, TreatAsOrdinal{});
            auto s2 = extractValue(src2, TreatAsOrdinal{});
            ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
        }
    }
    void
    Core::concmpi(RegLit src1, RegLit src2) {
        // don't care what the least significant two bits are of the cond code so just mask them out
        if ((ac.getConditionCode() & 0b100) == 0) {
            auto s1 = extractValue(src1, TreatAsInteger{});
            auto s2 = extractValue(src2, TreatAsInteger{});
            ac.setConditionCode(s1 <= s2 ? 0b010 : 0b000);
        }

    }
    void
    Core::cmpinco(RegLit src1, RegLit src2, RegisterIndex dest) {
        cmpo(src1, src2);
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(s2 + 1);
    }
    void
    Core::cmpinci(RegLit src1, RegLit src2, RegisterIndex dest) {
        cmpi(src1, src2);
        auto s2 = extractValue(src2, TreatAsInteger{});
        getRegister(dest).setInteger(s2 + 1); // manual states that this instruction suppresses overflow
    }
    void
    Core::cmpdeco(RegLit src1, RegLit src2, RegisterIndex dest) {
        cmpo(src1, src2);
        auto s2 = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(s2 - 1);
    }
    void
    Core::cmpdeci(RegLit src1, RegLit src2, RegisterIndex dest) {
        cmpi(src1, src2);
        auto s2 = extractValue(src2, TreatAsInteger{});
        getRegister(dest).setInteger(s2 - 1); // manual states that this instruction suppresses overflow
    }
}
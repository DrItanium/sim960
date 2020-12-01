#include "Core.h"
#include "DependentFalse.h"

namespace i960 {
    constexpr Ordinal computeSingleBitShiftMask(Ordinal value) noexcept {
        return 1 << (value & 0b11111);
    }
    Register&
    Core::getRegister(int index) noexcept {
        if (auto offset = index & 0b1111, maskedValue = index & 0b10000; maskedValue != 0) {
            return locals[offset];
        } else {
            return globals[offset];
        }
    }

    const Register&
    Core::getRegister(int index) const noexcept {
        if (auto offset = index & 0b1111, maskedValue = index & 0b10000; maskedValue != 0) {
            return locals[offset];
        } else {
            return globals[offset];
        }
    }

    void
    Core::memoryAccess() {

    }
    void
    Core::writeback() {

    }
    void
    Core::cycle() {
        executeInstruction(decodeInstruction(fetchInstruction()));
        memoryAccess();
        writeback();
    }
    void
    Core::saveLocals() noexcept
    {
        // okay, we have to save all of the registers to the stack or the on board
        // register cache (however, I'm not implementing that yet)
    }
    void
    Core::restoreLocals() noexcept
    {
        // restore the local register frame, generally done when you return from a
        // previous function
    }
    void
    Core::execute(const RegFormatInstruction& inst) noexcept {
        switch(inst.getOpcode()) {
            case 0x580: notbit(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x581: logicalAnd(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x582: andnot(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x583: setbit(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x584: notand(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                // case 0x585: break;
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
                //case 0x599: break;
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
                // case 0x610: atmod(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                // case 0x612: atadd(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x640: spanbit(inst.getSrc1(), inst.getDestination()); break;
            case 0x641: scanbit(inst.getSrc1(), inst.getDestination()); break;
            case 0x642:
                daddc(std::get<RegisterIndex>(inst.getSrc1()),
                      std::get<RegisterIndex>(inst.getSrc2()),
                      inst.getDestination());
                break;
            case 0x643:
                dsubc(std::get<RegisterIndex>(inst.getSrc1()),
                      std::get<RegisterIndex>(inst.getSrc2()),
                      inst.getDestination());
                break;
            case 0x644:
                dmovt(std::get<RegisterIndex>(inst.getSrc1()),
                      inst.getDestination());
                break;

                /// @todo inspect this one, the arguments are backwards
            case 0x645: // modac
                // this one is a little strange and has to be unpacked differently
                modac(inst);
                break;
            case 0x650: modify(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
            case 0x651: extract(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x654: modtc(inst); break;
                //case 0x655: modpc(inst); break;
                //case 0x658: intctl(inst.getSrc1(), inst.getDestination()); break;
                //case 0x659: sysctl(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x65B: icctl(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x65C: dcctl(inst.getSrc1(), inst.getSrc2(), inst.getDestination()); break;
                //case 0x660: calls(inst.getSrc1()); break;
                //case 0x66B: mark(); break;
                //case 0x66C: fmark(); break;
            case 0x66D: flushreg(); break;
                //case 0x66E: syncf(); break;
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
        }
    }
    void
    Core::execute(const COBRInstruction &inst) noexcept {
        switch (inst.getOpcode()) {
            case 0x300: bbc(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x370: bbs(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3A0: cmpibe(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3D0: cmpibne(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3C0: cmpibl(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3E0: cmpible(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x390: cmpibg(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
            case 0x3B0: cmpibge(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                //case 0x3F0: cmpibo(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
                //case 0x380: cmpibno(inst.getSrc1(), inst.getSrc2(), inst.getDisplacement()); break;
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
                //case 0x270: testo(inst.getSrc1()); break;
                //case 0x200: testno(inst.getSrc1()); break;
            default:
                /// @todo raise an error at this point
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
            case 0x100: bno(Displacement22{inst.getDisplacement()}); break;
            case 0x110: bg(Displacement22{inst.getDisplacement()}); break;
            case 0x120: be(Displacement22{inst.getDisplacement()}); break;
            case 0x130: bge(Displacement22{inst.getDisplacement()}); break;
            case 0x140: bl(Displacement22{inst.getDisplacement()}); break;
            case 0x150: bne(Displacement22{inst.getDisplacement()}); break;
            case 0x160: ble(Displacement22{inst.getDisplacement()}); break;
            case 0x170: bo(Displacement22{inst.getDisplacement()}); break;
                //case 0x180: faultno(); break;
                //case 0x190: faultg(); break;
                //case 0x1A0: faulte(); break;
                //case 0x1B0: faultge(); break;
                //case 0x1C0: faultl(); break;
                //case 0x1D0: faultne(); break;
                //case 0x1E0: faultle(); break;
                //case 0x1F0: faulto(); break;
            default:
                /// @todo raise illegal opcode fault
                break;
        }
    }
    Core::DecodedInstruction
    Core::decode(Ordinal value) noexcept {
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
    Core::DecodedInstruction
    Core::decodeInstruction(Ordinal currentInstruction) {
        return decode(currentInstruction);
    }
    void
    Core::executeInstruction(const Core::DecodedInstruction& inst) {
        std::visit([this](auto&& theInst) { execute(theInst); }, inst);
    }
    Ordinal
    Core::fetchInstruction() {
        return getWordAtIP(true);
    }

    void
    Core::addc(RegLit src1, RegLit src2, RegisterIndex dest) {
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
    Core::addi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{}) ;
        auto s2 = extractValue(src2, TreatAsInteger{}) ;
        getRegister(dest).setInteger(s2 + s1) ;
        /// @todo implement fault detection
    }

    void
    Core::addo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
        auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
        getRegister(dest).setOrdinal(s2 + s1) ;
        /// @todo implement fault detection
    }
    void
    Core::alterbit(RegLit bitpos, RegLit src, RegisterIndex dest) {
        /// @todo implement this body
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
    Core::divi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{}) ;
        auto s2 = extractValue(src2, TreatAsInteger{}) ;
        getRegister(dest).setInteger(s2 / s1) ;
        /// @todo implement fault detection
    }
    void
    Core::divo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
        auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
        getRegister(dest).setOrdinal(s2 / s1) ;
        /// @todo implement fault detection
    }

    void
    Core::modify(RegLit mask, RegLit src, RegisterIndex srcDest) {
        auto& sd = getRegister(srcDest);
        auto theMask = extractValue(mask, TreatAsOrdinal{});
        sd.setOrdinal((extractValue(src, TreatAsOrdinal{}) & theMask)  | (sd.getOrdinal() & (~theMask)));
    }

    void
    Core::muli(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{}) ;
        auto s2 = extractValue(src2, TreatAsInteger{}) ;
        getRegister(dest).setInteger(s2 * s1) ;
        /// @todo implement fault detection
    }
    void
    Core::mulo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
        auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
        getRegister(dest).setOrdinal(s2 * s1) ;
        /// @todo implement fault detection
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
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) & extractValue(src1,TreatAsOrdinal{}));
    }
    void
    Core::notor(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal((~extractValue(src2, TreatAsOrdinal{})) | extractValue(src1,TreatAsOrdinal{}));
    }
    void
    Core::logicalOr(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | extractValue(src1,TreatAsOrdinal{}));
    }
    void
    Core::ornot(RegLit src1, RegLit src2, RegisterIndex dest) {
        getRegister(dest).setOrdinal(extractValue(src2, TreatAsOrdinal{}) | (~extractValue(src1,TreatAsOrdinal{})));
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
    Core::subi(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsInteger{}) ;
        auto s2 = extractValue(src2, TreatAsInteger{}) ;
        getRegister(dest).setInteger(s2 - s1) ;
        /// @todo implement fault detection
    }
    void
    Core::subo(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = extractValue(src1, TreatAsOrdinal{}) ;
        auto s2 = extractValue(src2, TreatAsOrdinal{}) ;
        getRegister(dest).setOrdinal(s2 - s1) ;
        /// @todo implement fault detection
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
        } else {
            getRegister(dest).setOrdinal(extractValue(src, TreatAsOrdinal{}));
            getRegister(nextRegisterIndex(dest)).setOrdinal(extractValue(nextValue(src), TreatAsOrdinal{}));
            getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(extractValue(nextValue(nextValue(src)), TreatAsOrdinal{}));
            getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(extractValue(nextValue(nextValue(nextValue(src))), TreatAsOrdinal{}));
        }
    }
    void
    Core::begin() {
        theBoard.begin();
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

    Ordinal
    Core::getWordAtIP(bool advance) noexcept {
        auto ipLoc = ip.getOrdinal();
        if (advance) {
            ip.setOrdinal(ipLoc + 4);
        }
        return loadOrdinal(ipLoc);
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
        } else {
            storeOrdinal(address, getRegister(src).getOrdinal());
            storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
            if ((address & 0b111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED fault
            }
        }
    }
    void
    Core::stt(RegisterIndex src, Ordinal address) {
        if (!divisibleByFour(src)) {
            /// @todo raise a operation.invalid_operand fault
        } else {
            storeOrdinal(address, getRegister(src).getOrdinal());
            storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
            storeOrdinal(address + 8, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
            if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
            }
        }
    }

    void
    Core::stq(RegisterIndex src, Ordinal address) {
        if (!divisibleByFour(src)) {
            /// @todo raise a operation.invalid_operand fault
        } else {
            storeOrdinal(address, getRegister(src).getOrdinal());
            storeOrdinal(address + 4, getRegister(nextRegisterIndex(src)).getOrdinal());
            storeOrdinal(address + 8, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
            storeOrdinal(address + 12, getRegister(nextRegisterIndex(nextRegisterIndex(src))).getOrdinal());
            if ((address & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
            }
        }
    }

    void
    Core::ldl(Ordinal mem, RegisterIndex dest) {
        if(!divisibleByTwo(dest)) {
           /// @todo raise invalid_operand fault
           // the Hx docs state that dest is modified
           getRegister(dest).setOrdinal(-1);
        } else {
            getRegister(dest).setOrdinal(loadOrdinal(mem));
            getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
            if ((mem & 0b111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
            }
        }
    }
    void
    Core::ldt(Ordinal mem, RegisterIndex dest) {
        if(!divisibleByFour(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            getRegister(dest).setOrdinal(-1);
        } else {
            getRegister(dest).setOrdinal(loadOrdinal(mem));
            getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
            getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem+8));
            if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
            }
        }
    }
    void
    Core::ldq(Ordinal mem, RegisterIndex dest) {
        if(!divisibleByFour(dest)) {
            /// @todo raise invalid_operand fault
            // the Hx docs state that dest is modified
            getRegister(dest).setOrdinal(-1);
        } else {
            getRegister(dest).setOrdinal(loadOrdinal(mem));
            getRegister(nextRegisterIndex(dest)).setOrdinal(loadOrdinal(mem+4));
            getRegister(nextRegisterIndex(nextRegisterIndex(dest))).setOrdinal(loadOrdinal(mem+8));
            getRegister(nextRegisterIndex(nextRegisterIndex(nextRegisterIndex(dest)))).setOrdinal(loadOrdinal(mem+12));
            if ((mem & 0b1111) != 0 && _unalignedFaultEnabled) {
                /// @todo generate an OPERATION.UNALIGNED_FAULT
            }
        }
    }
    void
    Core::ret()
    {
        /// @todo implement
    }
    void
    Core::bno(Displacement22 dest) {
        if (ac.conditionIsUnordered()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::bo(Displacement22 dest) {
        if (ac.conditionIsOrdered()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::bg(Displacement22 dest) {
        if (ac.conditionIsGreaterThan()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::be(Displacement22 dest) {
        if (ac.conditionIsEqualTo()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }

    void
    Core::bge(Displacement22 dest) {
        if (ac.conditionIsGreaterThanOrEqualTo()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::bl(Displacement22 dest) {
        if (ac.conditionIsLessThan()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::bne(Displacement22 dest) {
        if (ac.conditionIsNotEqual()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::ble(Displacement22 dest) {
        if (ac.conditionIsLessThanOrEqual()) {
            ip.setInteger(ip.getInteger() + dest.getValue());
        }
    }
    void
    Core::balx(Ordinal targ, RegisterIndex dest) {
        /// @todo implement
    }
    void
    Core::ediv(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
    void
    Core::emul(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
    void
    Core::modi(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
    void
    Core::extract(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
    void
    Core::callx(Ordinal targ) {
    }
    void
    Core::call(Displacement22 targ) {

    }

    void
    Core::bx(Ordinal targ) {
        ip.setOrdinal(targ);
    }
    void
    Core::teste(RegisterIndex dest) {

    }
    void
    Core::testne(RegisterIndex dest) {

    }
    void
    Core::testl(RegisterIndex dest) {

    }
    void
    Core::testle(RegisterIndex dest) {

    }
    void
    Core::testg(RegisterIndex dest) {

    }
    void
    Core::testge(RegisterIndex dest) {

    }
    void
    Core::cmpibg(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpible(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpibe(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpibne(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpibl(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }
    void
    Core::cmpibge(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }
    void
    Core::cmpobg(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpoble(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpobe(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpobne(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }

    void
    Core::cmpobl(RegLit src1, RegisterIndex src2, ShortInteger targ) {

    }
    void
    Core::cmpobge(RegLit src1, RegisterIndex src2, ShortInteger targ) {

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
    Core::notbit(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
    void
    Core::flushreg() {

    }
    void
    Core::setbit(RegLit src1, RegLit src2, RegisterIndex dest) {

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
    Core::clrbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto bitpos = extractValue(src1, TreatAsOrdinal{});
        auto src = extractValue(src2, TreatAsOrdinal{});
        auto bitposModified = ~(computeSingleBitShiftMask(bitpos));
        getRegister(dest).setOrdinal(src & bitposModified);
    }
    void
    Core::rotate(RegLit src1, RegLit src2, RegisterIndex dest) {

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
    Core::cmpo(RegLit src1, RegLit src2) {
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
    Core::cmpi(RegLit src1, RegLit src2) {
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
    Core::concmpo(RegLit src1, RegLit src2) {

    }
    void
    Core::concmpi(RegLit src1, RegLit src2) {

    }
    void
    Core::scanbyte(RegLit src1, RegLit src2) {

    }
    void
    Core::chkbit(RegLit src1, RegLit src2) {
        ac.setConditionCode(((extractValue(src2, TreatAsOrdinal{})& computeSingleBitShiftMask(extractValue(src1, TreatAsOrdinal{}))) == 0) ? 0b000 : 0b010);
    }
    void
    Core::spanbit(RegLit src1, RegisterIndex src2) {

    }
    void
    Core::scanbit(RegLit src1, RegisterIndex src2) {

    }
    void
    Core::dsubc(RegisterIndex src1, RegisterIndex src2, RegisterIndex dest) {

    }
    void
    Core::dmovt(RegisterIndex src1, RegisterIndex dest) {

    }
    void
    Core::shri(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
    void
    Core::shrdi(RegLit src1, RegLit src2, RegisterIndex dest) {

    }
} // end namespace i960

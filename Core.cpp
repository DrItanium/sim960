#include "Core.h"

namespace i960 {
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
    Core::fetchInstruction() {

    }
    void
    Core::decodeInstruction() {

    }
    void
    Core::executeInstruction() {

    }
    void
    Core::memoryAccess() {

    }
    void
    Core::writeback() {

    }
    void
    Core::cycle() {
       fetchInstruction();
       decodeInstruction();
       executeInstruction();
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
    Core::addc(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto s1 = static_cast<LongOrdinal>(extractValue(src1, TreatAsOrdinal{}));
        auto s2 = static_cast<LongOrdinal>(extractValue(src2, TreatAsOrdinal{}));
        auto result = s2 + s1;
        auto upperHalf = static_cast<Ordinal>(result >> 32);
        getRegister(dest).setOrdinal(static_cast<Ordinal>(result));
        /// @todo update condition codes to propagate carry and if integer addition would've generate an overflow
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
    Core::b(Displacement targ) {
        ip.setInteger(ip.getInteger() + targ.getValue());
    }
    void
    Core::bal(Displacement targ) {
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
} // end namespace i960

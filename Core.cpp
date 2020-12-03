#include "Core.h"
#include "DependentFalse.h"

#include <algorithm>

namespace i960 {



    void
    Core::extract(RegLit src1, RegLit src2, RegisterIndex dest) {
        // taken from the i960Hx manual
        getRegister(dest).setOrdinal((extractValue(dest, TreatAsOrdinal{}) >> std::min(extractValue(src1, TreatAsOrdinal{}), static_cast<Ordinal>(32))) &
                                     (~(0xFFFF'FFFF << extractValue(src2, TreatAsOrdinal{}))));
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
    Core::cmpibg(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpi(src1, src2);
        bg(Displacement22{targ});
    }

    void
    Core::cmpible(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpi(src1, src2);
        ble(Displacement22{targ});

    }

    void
    Core::cmpibe(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpi(src1, src2);
        be(Displacement22{targ});

    }

    void
    Core::cmpibne(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpi(src1, src2);
        bne(Displacement22{targ});
    }

    void
    Core::cmpibl(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpi(src1, src2);
        bl(Displacement22{targ});
    }
    void
    Core::cmpibge(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpi(src1, src2);
        bge(Displacement22{targ});
    }
    void
    Core::cmpobg(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpo(src1, src2);
        bg(Displacement22{targ});
    }

    void
    Core::cmpoble(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpo(src1, src2);
        ble(Displacement22{targ});
    }

    void
    Core::cmpobe(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpo(src1, src2);
        be(Displacement22{targ});
    }

    void
    Core::cmpobne(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpo(src1, src2);
        bne(Displacement22{targ});
    }

    void
    Core::cmpobl(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpo(src1, src2);
        bl(Displacement22{targ});
    }
    void
    Core::cmpobge(RegLit src1, RegisterIndex src2, ShortInteger targ) {
        cmpo(src1, src2);
        bge(Displacement22{targ});
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
    Core::flushreg() {

    }
    void
    Core::setbit(RegLit src1, RegLit src2, RegisterIndex dest) {
        auto bitpos = extractValue(src1, TreatAsOrdinal{}) ;
        auto src = extractValue(src2, TreatAsOrdinal{});
        getRegister(dest).setOrdinal(src | (1 << (bitpos & 0b11111)));
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
        auto len = extractValue(src1, TreatAsOrdinal {});
        auto src = extractValue(src2, TreatAsOrdinal {});
        getRegister(dest).setOrdinal(rotateOperation(src, len));
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
    void
    Core::chkbit(RegLit src1, RegLit src2) {
        ac.setConditionCode(((extractValue(src2, TreatAsOrdinal{})& computeSingleBitShiftMask(extractValue(src1, TreatAsOrdinal{}))) == 0) ? 0b000 : 0b010);
    }
    constexpr Ordinal largestOrdinal = 0xFFFF'FFFF;
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
    Core::syncf() {
        if (ac.getNoImpreciseFaults()) {
            return;
        }
        // do a noop
    }
    void
    Core::cmpibo(RegLit src1, RegLit src2, ShortInteger targ) {
        cmpi(src1, src2);
        bo(Displacement22{targ});
    }
    void
    Core::cmpibno(RegLit src1, RegLit src2, ShortInteger targ) {
        cmpi(src1, src2);
        bno(Displacement22{targ});
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
    Core::eshro(RegLit src1, RegLit src2, RegisterIndex dest) {

    }

    void
    Core::ediv(RegLit src1, RegLit src2, RegisterIndex dest) {

    }

    void
    Core::emul(RegLit src1, RegLit src2, RegisterIndex dest) {

    }

    void
    Core::shri(RegLit src1, RegLit src2, RegisterIndex dest) {

    }

    void
    Core::shrdi(RegLit src1, RegLit src2, RegisterIndex dest) {

    }

} // end namespace i960

//
// Created by jwscoggins on 12/5/20.
//

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

}
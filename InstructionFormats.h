//
// Created by jwscoggins on 12/21/20.
//

#ifndef SIM960_INSTRUCTIONFORMATS_H
#define SIM960_INSTRUCTIONFORMATS_H
#include <string>
#include "CoreTypes.h"
#include "Register.h"
#include "DependentFalse.h"
#include "ArithmeticControls.h"
#include "ProcessControls.h"
#include "Disassembly.h"
#include "TraceControls.h"
#include "PreviousFramePointer.h"
#include "BusInterfaceUnit.h"
namespace i960 {
    class RegFormatInstruction {
    public:
        constexpr RegFormatInstruction(Ordinal value) noexcept: _value(value) {}
        constexpr i960::RegLit getSrc1() const noexcept {
            // no sfr check
            if (m1) {
                return i960::toLiteral(src1);
            } else {
                return i960::toRegisterIndex(src1);
            }
        }
        constexpr i960::RegLit getSrc2() const noexcept {
            // no sfr check
            if (m2) {
                return i960::toLiteral(src2);
            } else {
                return i960::toRegisterIndex(src2);
            }
        }
        constexpr i960::RegisterIndex getDestination() const noexcept {
            return i960::toRegisterIndex(srcDest);
        }
        constexpr i960::RegLit getSrcDest() const noexcept {
            if (!m3) {
                return getDestination();
            } else {
                // can be a literal when src/dest is used as a source
                return i960::toLiteral(srcDest);
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
                unsigned int src1: 5;
                unsigned int s1: 1;
                unsigned int s2: 1;
                unsigned int opcodeExt: 4;
                unsigned int m1: 1;
                unsigned int m2: 1;
                unsigned int m3: 1;
                unsigned int src2: 5;
                unsigned int srcDest: 5;
                unsigned int opcode: 8;
            };
        };
    };

    class COBRInstruction {
    public:
        constexpr COBRInstruction(Ordinal value) noexcept: _value(value) {}
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr i960::RegisterIndex getSrc2() const noexcept { return i960::toRegisterIndex(src2); }
        constexpr i960::RegisterIndex getSrc1() const noexcept { return i960::toRegisterIndex(src1); }
        constexpr bool getTBit() const noexcept { return t; }
        constexpr ShortInteger getDisplacement() const noexcept { return displacement; }
        std::string decodeName() const noexcept;
        constexpr auto lowerHalf() const noexcept { return _value; }
    private:
        union {
            Ordinal _value;
            struct {
                unsigned int s2: 1;
                unsigned int t: 1;
                int displacement: 11;
                unsigned int m1: 1;
                unsigned int src2: 5;
                unsigned int src1: 5;
                unsigned int opcode: 8;
            };
        };
    };

    class CTRLInstruction {
    public:
        constexpr CTRLInstruction(Ordinal value) noexcept: _value(value) {}
        constexpr ShortOrdinal getOpcode() const noexcept { return (static_cast<ShortOrdinal>(opcode) << 4) & 0x0FF0; }
        constexpr bool getTBit() const noexcept { return _value & 0b10; }
        constexpr Integer getDisplacement() const noexcept { return displacement & 0x00FF'FFFC; }
        std::string decodeName() const noexcept;
        constexpr auto lowerHalf() const noexcept { return _value; }
    private:
        union {
            Ordinal _value;
            struct {
                int displacement: 24;
                unsigned int opcode: 8;
            };
        };
    };

    class MEMFormatInstruction {
    public:
        constexpr explicit MEMFormatInstruction(Ordinal lowerHalf, Ordinal upperHalf) noexcept: lower(lowerHalf), next(upperHalf) {}
        [[nodiscard]] constexpr ShortOrdinal getOpcode() const noexcept { return static_cast<ShortOrdinal>(opcode) << 4; }
        template<typename Core>
        Ordinal computeAddress(Core &referenceCore) const noexcept {
            if (isMEMAFormat()) {
                return computeAddress_MEMA(referenceCore);
            } else {
                return computeAddress_MEMB(referenceCore);
            }
        }
        [[nodiscard]] constexpr i960::RegisterIndex getSrcDest() const noexcept { return static_cast<i960::RegisterIndex>(srcDest); }
        std::string decodeName() const noexcept;
        [[nodiscard]] constexpr auto upperHalf() const noexcept { return next; }
        [[nodiscard]] constexpr auto lowerHalf() const noexcept { return lower; }
    private:
        [[nodiscard]] constexpr bool isMEMAFormat() const noexcept { return (modeMajor & 1u) == 0; }
        [[nodiscard]] constexpr bool isMEMBFormat() const noexcept { return (modeMajor & 1u) != 0; }
        template<typename Core>
        [[nodiscard]] Ordinal computeAddress_MEMA(Core &referenceCore) const noexcept {
            // the lsb of mema.mode will always be 0 to get to this point
            if (mema.mode == 0b00) {
                // absolute offset
                return static_cast<Ordinal>(mema.offset);
            } else {
                // register indirect with offset
                return static_cast<Ordinal>(mema.offset) + referenceCore.getRegister(i960::toRegisterIndex(abase)).getOrdinal();
            }

        }
        template<typename Core>
        Ordinal computeAddress_MEMB(Core &referenceCore) const noexcept {
            // bit 12 of the instruction has to be 1 to get into this code, eight possible combos
            switch (memb.mode) {
                case 0b0100: // register indirect
                    return referenceCore.getRegister(i960::toRegisterIndex(memb.abase)).getOrdinal();
                case 0b0101: // ip with displacement
                    referenceCore.instructionIsDoubleWide();
                    return static_cast<Ordinal>(referenceCore.getIP().getInteger() + optionalDisplacement + 8);
                case 0b0110: // reserved
                    return -1;
                case 0b0111: // register indirect with index
                    return referenceCore.getRegister(i960::toRegisterIndex(memb.abase)).getOrdinal() +
                           referenceCore.getRegister(i960::toRegisterIndex(memb.index)).getOrdinal() *
                           computeScale(referenceCore);
                case 0b1100: // absolute displacement
                    referenceCore.instructionIsDoubleWide();
                    return static_cast<Ordinal>(optionalDisplacement);
                case 0b1101: // register indirect with displacement
                    referenceCore.instructionIsDoubleWide();
                    return static_cast<Ordinal>(referenceCore.getRegister(i960::toRegisterIndex(memb.abase)).getInteger() +
                                                optionalDisplacement);
                case 0b1110: // index with displacement
                    referenceCore.instructionIsDoubleWide();
                    return static_cast<Ordinal>(referenceCore.getRegister(i960::toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) + optionalDisplacement);
                case 0b1111: // register indirect with index and displacement
                    referenceCore.instructionIsDoubleWide();
                    return static_cast<Ordinal>(referenceCore.getRegister(i960::toRegisterIndex(memb.abase)).getInteger() +
                                                referenceCore.getRegister(i960::toRegisterIndex(memb.index)).getInteger() *
                                                computeScale(referenceCore) + optionalDisplacement);
                default:
                    return -1;
            }
        }
        template<typename Core>
        Ordinal computeScale(Core &referenceCore) const noexcept {

            switch (memb.scale) {
                case 0b000:
                    return 1;
                case 0b001:
                    return 2;
                case 0b010:
                    return 4;
                case 0b011:
                    return 8;
                case 0b100:
                    return 16;
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
                unsigned int differentiationBlock: 12;
                unsigned int modeMajor: 2;
                unsigned int abase: 5;
                unsigned int srcDest: 5;
                unsigned int opcode: 8;
            };
            struct {
                unsigned int offset: 12;
                unsigned int mode: 2;
                unsigned int abase: 5;
                unsigned int srcDest: 5;
                unsigned int opcode: 8;
            } mema;
            struct {
                unsigned int index: 5;
                unsigned int unused0: 2;
                unsigned int scale: 3;
                unsigned int mode: 4;
                unsigned int abase: 5;
                unsigned int srcDest: 5;
                unsigned int opcode: 8;
            } memb;
        };
        union {
            Ordinal next;
            Integer optionalDisplacement;
        };
    };

}
#endif //SIM960_INSTRUCTIONFORMATS_H

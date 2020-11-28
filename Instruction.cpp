//
// Created by jwscoggins on 11/28/20.
//
#include "Core.h"

namespace i960 {
    Ordinal
    MEMFormatInstruction::computeAddress(Core& referenceCore) const noexcept {
        if (isMEMAFormat()) {
            return computeAddress_MEMA(referenceCore);
        } else {
            return computeAddress_MEMB(referenceCore);
        }
    }

    Ordinal
    MEMFormatInstruction::computeAddress_MEMA(Core &referenceCore) const noexcept {
        // the lsb of mema.mode will always be 0 to get to this point
        if (mema.mode == 0b00) {
            // absolute offset
            return static_cast<Ordinal>(mema.offset);
        } else {
            // register indirect with offset
            return static_cast<Ordinal>(mema.offset) + referenceCore.getRegister(toRegisterIndex(abase)).getOrdinal();
        }
    }

    Ordinal
    MEMFormatInstruction::computeScale(Core &referenceCore) const noexcept {
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

    Ordinal
    MEMFormatInstruction::computeAddress_MEMB(Core &referenceCore) const noexcept {
        // bit 12 of the instruction has to be 1 to get into this code, eight possible combos
        switch (memb.mode) {
            case 0b0100: // register indirect
                return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal();
            case 0b0101: // ip with displacement
                return static_cast<Ordinal>(referenceCore.getIP().getInteger() + optionalDisplacement + 8);
            case 0b0110: // reserved
                return -1;
            case 0b0111: // register indirect with index
                return referenceCore.getRegister(toRegisterIndex(memb.abase)).getOrdinal() +
                       referenceCore.getRegister(toRegisterIndex(memb.index)).getOrdinal() *
                       computeScale(referenceCore);
            case 0b1100: // absolute displacement
                return static_cast<Ordinal>(optionalDisplacement);
            case 0b1101: // register indirect with displacement
                return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                            optionalDisplacement);
            case 0b1110: // index with displacement
                return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                            computeScale(referenceCore) +
                                            optionalDisplacement);
            case 0b1111: // register indirect with index and displacement
                return static_cast<Ordinal>(referenceCore.getRegister(toRegisterIndex(memb.abase)).getInteger() +
                                            referenceCore.getRegister(toRegisterIndex(memb.index)).getInteger() *
                                            computeScale(referenceCore) +
                                            optionalDisplacement);
            default:
                return -1;
        }
    }

}

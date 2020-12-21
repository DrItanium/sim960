//
// Created by jwscoggins on 12/21/20.
//

#include "InstructionFormats.h"

namespace i960 {

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
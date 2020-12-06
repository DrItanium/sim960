//
// Created by jwscoggins on 12/5/20.
//

#ifndef SIM960_INSTRUCTIONNAMES_H
#define SIM960_INSTRUCTIONNAMES_H
#include <map>
#include <string>
#include <optional>
#include "CoreTypes.h"
namespace i960 {
    std::optional<std::string> getNameFromOpcode(ShortOrdinal opcode) noexcept;
    std::optional<ShortOrdinal> getOpcodeFromName(const std::string& name) noexcept;
}
#endif //SIM960_INSTRUCTIONNAMES_H

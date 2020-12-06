//
// Created by jwscoggins on 12/5/20.
//

#include "InstructionNames.h"

namespace i960 {
    std::optional<std::string>
    getNameFromOpcode(ShortOrdinal opcode) noexcept {
        static const std::map<ShortOrdinal, std::string> table {
#define X(opcode, name) { static_cast<ShortOrdinal>(opcode), #name },
#include "Opcodes.def"
#undef X
        };
        if (auto result = table.find(opcode); result != table.cend()) {
            return std::make_optional(result->second);
        } else {
            return std::nullopt;
        }
    }

    std::optional<ShortOrdinal>
    getOpcodeFromName(const std::string& name) noexcept {
        static const std::map<std::string, ShortOrdinal> table {
#define X(opcode, name) { #name, static_cast<ShortOrdinal>(opcode) },
#include "Opcodes.def"
#undef X
        };
        if (auto result = table.find(name); result != table.cend()) {
            return std::make_optional(result->second);
        } else {
            return std::nullopt;
        }
    }

}


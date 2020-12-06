//
// Created by jwscoggins on 12/5/20.
//

#ifndef SIM960_DISASSEMBLY_H
#define SIM960_DISASSEMBLY_H
#include <map>
#include <string>
#include <optional>
#include "CoreTypes.h"
namespace i960 {
    inline std::optional<std::string> getNameFromOpcode(ShortOrdinal opcode) noexcept {
        static const std::map<ShortOrdinal, std::string> table {
#define X(opcode, name) { static_cast<ShortOrdinal>(opcode), #name },
#include "OpcodesRaw.h"
#undef X
        };
        if (auto result = table.find(opcode); result != table.cend()) {
            return std::make_optional(result->second);
        } else {
            return std::nullopt;
        }
    }

    inline std::optional<ShortOrdinal> getOpcodeFromName(const std::string& name) noexcept {
        static const std::map<std::string, ShortOrdinal> table {
#define X(opcode, name) { #name, static_cast<ShortOrdinal>(opcode) },
#include "OpcodesRaw.h"
#undef X
        };
        if (auto result = table.find(name); result != table.cend()) {
            return std::make_optional(result->second);
        } else {
            return std::nullopt;
        }
    }
}
#endif //SIM960_DISASSEMBLY_H

//
// Created by jwscoggins on 12/12/20.
//

#include "ProcessorMappingConfiguration.h"

namespace i960 {
    bool BlockConfiguration::isUnmapped() const noexcept { return type == "unmapped"; }
    bool BlockConfiguration::isSd() const noexcept { return type == "sd"; }
    bool BlockConfiguration::isOnChip() const noexcept { return type == "onchip"; }
    bool BlockConfiguration::isExternalIO() const noexcept { return type == "external-io"; }

    BlockConfiguration::BlockConfiguration(const std::string &kind,
                                           const std::string &desc,
                                           const std::string &path,
                                           const std::string &permissions)
            : type(kind), description(desc), filename(path), perms(permissions) { }



    BlockConfiguration&
    MappingConfiguration::get(size_t index) {
        return blocks[index&0xFF];
    }
    const BlockConfiguration&
    MappingConfiguration::get(size_t index) const {
        return blocks[index&0xFF];
    }

}
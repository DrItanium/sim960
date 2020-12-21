//
// Created by jwscoggins on 12/12/20.
//

#include "ProcessorMappingConfiguration.h"

namespace i960 {
    bool SectionConfiguration::isUnmapped() const noexcept { return type == "unmapped"; }
    bool SectionConfiguration::isSd() const noexcept { return type == "sd"; }
    bool SectionConfiguration::isOnChip() const noexcept { return type == "onchip"; }
    bool SectionConfiguration::isExternalIO() const noexcept { return type == "external-io"; }

    SectionConfiguration::SectionConfiguration(const std::string &kind,
                                               const std::string &desc,
                                               const std::string &path,
                                               const std::string &permissions)
            : type(kind), description(desc), filename(path), perms(permissions) { }



    SectionConfiguration&
    MappingConfiguration::get(size_t index) {
        return sections_[index & 0xFF];
    }
    const SectionConfiguration&
    MappingConfiguration::get(size_t index) const {
        return sections_[index & 0xFF];
    }

}
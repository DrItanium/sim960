//
// Created by jwscoggins on 12/12/20.
//

#ifndef SIM960_PROCESSORMAPPINGCONFIGURATION_H
#define SIM960_PROCESSORMAPPINGCONFIGURATION_H
#include <string>
#include <array>
namespace i960 {
    struct BlockConfiguration {
    public:
        BlockConfiguration() = default;
        BlockConfiguration(const std::string& kind, const std::string& desc, const std::string& path, const std::string& permissions);
        [[nodiscard]] const std::string& getPermissions() const noexcept { return perms; }
        [[nodiscard]] const std::string& getFilename() const noexcept { return filename; }
        [[nodiscard]] const std::string& getDescription() const noexcept { return description; }
        [[nodiscard]] const std::string& getType() const noexcept { return type; }
        [[nodiscard]] bool isUnmapped() const noexcept;
        [[nodiscard]] bool isSd() const noexcept;
        [[nodiscard]] bool isOnChip() const noexcept ;
        [[nodiscard]] bool isExternalIO() const noexcept;
        void setPermissions(const std::string& value) noexcept { perms = value; }
        void setDescription(const std::string& value) noexcept { description = value;}
        void setFilename(const std::string& value) noexcept { filename = value;}
        void setType(const std::string& value) noexcept { type = value; }
    private:
        std::string type;
        std::string description;
        std::string filename;
        std::string perms;
    };

    struct MappingConfiguration {
    public:
        MappingConfiguration() = default;
        void setName(const std::string& name) noexcept;
        [[nodiscard]] decltype(auto) begin() const { return blocks.begin(); }
        [[nodiscard]] decltype(auto) begin() { return blocks.begin(); }
        [[nodiscard]] decltype(auto) end() const { return blocks.end(); }
        [[nodiscard]] decltype(auto) end() { return blocks.end(); }
        [[nodiscard]] BlockConfiguration& get(size_t index);
        [[nodiscard]] const BlockConfiguration& get(size_t index) const;
        [[nodiscard]] const std::string& getName() const noexcept { return name; }
    private:
        std::string name;
        std::array<BlockConfiguration, 256> blocks;
    };

}

#endif //SIM960_PROCESSORMAPPINGCONFIGURATION_H
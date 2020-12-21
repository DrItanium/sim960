//
// Created by jwscoggins on 12/20/20.
//

#ifndef SIM960_PROCESSORADDRESS_H
#define SIM960_PROCESSORADDRESS_H
#include "CoreTypes.h"

namespace i960 {
    /**
     * @brief Wraps a normal 32-bit Ordinal/Address and allows access to the different components.
     * Each address is made up of four components (in order from MSB to LSB): Section, Subsection, Block
     * There are 256 Sections.
     * Each section contains 256 subsections.
     * Each subsection contains 256 blocks.
     * Each Block contains 256 Sectors.
     *
     * In terms of bytes:
     * 1 Section = 16 Mbytes
     * 1 Subsection = 64 Kbytes
     * 1 Block = 256 bytes
     */
    class ProcessorAddress {
    public:
        constexpr explicit ProcessorAddress(Address value) noexcept : full_(value) { }
        constexpr ProcessorAddress(ByteOrdinal offset, ByteOrdinal blockId, ByteOrdinal subsectionId, ByteOrdinal sectionId) noexcept :
        ProcessorAddress(static_cast<Address>(offset) |
                         static_cast<Address>(blockId) << 8 |
                         static_cast<Address>(subsectionId) << 16 |
                         static_cast<Address>(sectionId) << 24) { }
        [[nodiscard]] constexpr ByteOrdinal getBlockOffset() const noexcept { return full_; }
        [[nodiscard]] constexpr ByteOrdinal getBlockId() const noexcept { return full_ >> 8; }
        [[nodiscard]] constexpr ByteOrdinal getSubsectionId() const noexcept { return full_ >> 16; }
        [[nodiscard]] constexpr ByteOrdinal getSectionId() const noexcept { return full_ >> 24; }
    private:
        Address full_;
    };
    static_assert(ProcessorAddress(0x01020304).getBlockOffset() == 0x04);
    static_assert(ProcessorAddress(0x01020304).getBlockId() == 0x03);
    static_assert(ProcessorAddress(0x01020304).getSubsectionId() == 0x02);
    static_assert(ProcessorAddress(0x01020304).getSectionId() == 0x01);
    static_assert(ProcessorAddress(0x04,0x03,0x02,0x01).getBlockOffset() == 0x04);
    static_assert(ProcessorAddress(0x04,0x03,0x02,0x01).getBlockId() == 0x03);
    static_assert(ProcessorAddress(0x04,0x03,0x02,0x01).getSubsectionId() == 0x02);
    static_assert(ProcessorAddress(0x04,0x03,0x02,0x01).getSectionId() == 0x01);
} // end namespace i960
#endif //SIM960_PROCESSORADDRESS_H

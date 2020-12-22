//
// Created by jwscoggins on 12/22/20.
//

#ifndef SIM960_MEMORYCELL_H
#define SIM960_MEMORYCELL_H
#include "CoreTypes.h"
namespace i960 {
    /**
     * @brief Generic concept of a 32-bit memory location
     */
    class MemoryCell {
        public:
        MemoryCell() = default;
        virtual ~MemoryCell() = default;
        /**
         * @brief Retrieve the full set of bits stored in this memory cell; override to change logic
         * @return the bits stored in this memory cell
         */
        [[nodiscard]] virtual Ordinal get() const noexcept { return 0; }
        /**
         * @brief Set the bits within this memory cell (do nothing by default)
         * @param value the bits to stash into this memory cell
         */
        virtual void set(Ordinal value) noexcept { }
    };
}

#endif //SIM960_MEMORYCELL_H

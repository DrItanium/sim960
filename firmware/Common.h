//
// Created by jwscoggins on 12/22/20.
//

#ifndef SIM960_COMMON_H
#define SIM960_COMMON_H
#include <stdint.h>
namespace firmware {

    template<typename T>
    volatile T& memory(const uint32_t address) {
        return *reinterpret_cast<T*>(address);
    }
} // end namespace firmware
#endif //SIM960_COMMON_H

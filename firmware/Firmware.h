//
// Created by jwscoggins on 12/22/20.
//

#ifndef SIM960_COMMON_H
#define SIM960_COMMON_H
#include <stdint.h>
#include <string>
typedef uint32_t Address;
namespace firmware {
    const Address IOSpaceStart = 0xFF000000;
    const Address InternalRegistersStart = 0xFFFF0000;
    const Address InternalRegistersEnd = 0xFFFFFFFF;
    template<typename T>
    volatile T& memory(const Address address) {
        return *reinterpret_cast<T*>(address);
    }
    volatile uint32_t& ioSpaceRegister(Address offset);
    volatile uint32_t& internalRegister(uint16_t offset);

} // end namespace firmware
void runApplication();
extern "C" {
    void bootup(void);
}
#endif //SIM960_COMMON_H

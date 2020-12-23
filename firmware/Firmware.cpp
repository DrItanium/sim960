//
// Created by jwscoggins on 12/22/20.
//

#include "Firmware.h"

namespace firmware
{
    volatile uint32_t&
    ioSpaceRegister(Address offset) {
        uint32_t fixed = 0x00FFFFFF & offset;
        return memory<uint32_t>(fixed | IOSpaceStart);
    }
    volatile uint32_t&
    internalRegister(uint16_t offset) {
        uint32_t fixed = 0x00FF0000 | offset;
        return ioSpaceRegister(fixed);
    }

}

volatile void
delay(uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {

    }
}
volatile void delay() { delay(firmware::internalRegister(0x104)) ; }
volatile void
setLED(bool value) {
    firmware::internalRegister(0x100) = value ? 0xFFFFFFFF : 0;
}
int
__main(int, char**) {
    while (true) {
        setLED(true);
        delay();
        setLED(false);
        delay();
    }
    return 0;
}
extern "C" {
void
_exit(void) {

}
int
brk(void* addr) {
    return 0;
}
void*
sbrk(intptr_t increment) {
    return 0;
}

void
start() {
    __main(0, 0);
    while (true);
}

}


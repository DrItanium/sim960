//
// Created by jwscoggins on 12/22/20.
//

#include "Firmware.h"
#include <errno.h>
void doIt();
extern "C" volatile void start() {
    doIt();
}
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
extern "C" {
void
_exit(void) {

}
intptr_t brkSize = 0;
const intptr_t maximumDataAddress = 0x03FFFFFF;

int
brk(void* addr) {
    if (reinterpret_cast<intptr_t>(addr) <= maximumDataAddress) {
        brkSize = reinterpret_cast<intptr_t>(addr);
        return 0;
    } else {
        errno = ENOMEM;
        return -1;
    }
}
void*
sbrk(intptr_t increment) {
    void* oldBrkSize = reinterpret_cast<void*>(brkSize);
    intptr_t addr = increment + brkSize;
    if (addr <= maximumDataAddress) {
        return oldBrkSize;
    } else {
        errno = ENOMEM;
        return reinterpret_cast<void *>(-1);
    }
}


}

void
doIt() {
    while (true) {
        setLED(true);
        delay();
        setLED(false);
        delay();
    }
}

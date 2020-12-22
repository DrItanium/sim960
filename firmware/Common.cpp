//
// Created by jwscoggins on 12/22/20.
//

#include "Common.h"

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

extern "C" {
    void bootup(void) {

    }
}





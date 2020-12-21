//
// Created by jwscoggins on 12/21/20.
//

#ifndef SIM960_INTERNALPERIPHERALUNIT_H
#define SIM960_INTERNALPERIPHERALUNIT_H
#include "CoreTypes.h"
#include "InterfaceUnit.h"
namespace i960 {
    /**
     * @brief Describes a device which will interact with the internal peripherals that the processor provides
     */
    class InternalPeripheralUnit : public InterfaceUnit {
    public:
        using InterfaceUnit::InterfaceUnit;
        ~InternalPeripheralUnit() override = default;
    };
}
#endif //SIM960_INTERNALPERIPHERALUNIT_H

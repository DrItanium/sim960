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
        virtual void begin() noexcept = 0;
        ByteOrdinal load(Address address, TreatAsByteOrdinal ordinal) override { return 0; }
        ByteInteger load(Address address, TreatAsByteInteger integer) override { return 0; }
        ShortOrdinal load(Address address, TreatAsShortOrdinal ordinal) override { return 0; }
        ShortInteger load(Address address, TreatAsShortInteger integer) override { return 0; }
        Integer load(Address address, TreatAsInteger integer) override { return 0; }
        void store(Address address, ByteOrdinal value, TreatAsByteOrdinal ordinal) override { }
        void store(Address address, ByteInteger value, TreatAsByteInteger integer) override { }
        void store(Address address, ShortOrdinal value, TreatAsShortOrdinal ordinal) override { }
        void store(Address address, ShortInteger value, TreatAsShortInteger integer) override { }
        void store(Address address, Integer value, TreatAsInteger integer) override { }
    };
}
#endif //SIM960_INTERNALPERIPHERALUNIT_H

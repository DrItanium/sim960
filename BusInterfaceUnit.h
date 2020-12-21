//
// Created by jwscoggins on 12/21/20.
//

#ifndef SIM960_BUSINTERFACEUNIT_H
#define SIM960_BUSINTERFACEUNIT_H
#include "CoreTypes.h"
namespace i960 {
    class BusInterfaceUnit {
    public:
        BusInterfaceUnit() = default;
        virtual ~BusInterfaceUnit() = default;
        virtual ByteOrdinal load(Address address, TreatAsByteOrdinal) = 0;
        virtual ByteInteger load(Address address, TreatAsByteInteger) = 0;
        virtual ShortOrdinal load(Address address, TreatAsShortOrdinal) = 0;
        virtual ShortInteger load(Address address, TreatAsShortInteger) = 0;
        virtual Ordinal load(Address address, TreatAsOrdinal) = 0;
        virtual Integer load(Address address, TreatAsInteger) = 0;
        virtual void store(Address address, ByteOrdinal value, TreatAsByteOrdinal) = 0;
        virtual void store(Address address, ByteInteger value, TreatAsByteInteger) = 0;
        virtual void store(Address address, ShortOrdinal value, TreatAsShortOrdinal) = 0;
        virtual void store(Address address, ShortInteger value, TreatAsShortInteger) = 0;
        virtual void store(Address address, Ordinal value, TreatAsOrdinal) = 0;
        virtual void store(Address address, Integer value, TreatAsInteger) = 0;
    };
} // end namespace i960

#endif //SIM960_BUSINTERFACEUNIT_H

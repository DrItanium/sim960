//
// Created by jwscoggins on 12/21/20.
//

#ifndef SIM960_INTERFACEUNIT_H
#define SIM960_INTERFACEUNIT_H
#include "CoreTypes.h"
namespace i960 {
    class InterfaceUnit {
    public:
        InterfaceUnit() = default;
        virtual ~InterfaceUnit() = default;
        virtual ByteOrdinal load(Address address, TreatAsByteOrdinal) = 0;
        virtual ByteInteger load(Address address, TreatAsByteInteger) = 0;
        virtual ShortOrdinal load(Address address, TreatAsShortOrdinal) = 0;
        virtual ShortInteger load(Address address, TreatAsShortInteger) = 0;
        virtual Ordinal load(Address address, TreatAsOrdinal) = 0;
        virtual void store(Address address, ByteOrdinal value, TreatAsByteOrdinal) = 0;
        virtual void store(Address address, ByteInteger value, TreatAsByteInteger) = 0;
        virtual void store(Address address, ShortOrdinal value, TreatAsShortOrdinal) = 0;
        virtual void store(Address address, ShortInteger value, TreatAsShortInteger) = 0;
        virtual void store(Address address, Ordinal value, TreatAsOrdinal) = 0;
    };
} // end namespace i960

#endif //SIM960_INTERFACEUNIT_H

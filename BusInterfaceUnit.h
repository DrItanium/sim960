//
// Created by jwscoggins on 12/21/20.
//

#ifndef SIM960_BUSINTERFACEUNIT_H
#define SIM960_BUSINTERFACEUNIT_H
#include "CoreTypes.h"
#include "InterfaceUnit.h"
namespace i960 {
   class BusInterfaceUnit : public InterfaceUnit {
   public:
       using InterfaceUnit::InterfaceUnit;
       ~BusInterfaceUnit() override = default;
   };
} // end namespace i960
#endif //SIM960_BUSINTERFACEUNIT_H

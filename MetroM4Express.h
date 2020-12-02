//
// Created by jwscoggins on 11/26/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_METROM4EXPRESS_H
#define I960_PROTOTYPE_SIMULATOR_METROM4EXPRESS_H
#if defined(ADAFRUIT_METRO_M4_EXPRESS) || defined(ARDUINO_METRO_M4)
#include "MemoryInterface.h"
#include "HasOnboardNeoPixel.h"
class MetroM4ExpressBoard : public MemoryInterface, public HasOnboardNeoPixel {
public:
    MetroM4ExpressBoard();
    ~MetroM4ExpressBoard() override = default;
    Integer loadValue(Address address, TreatAsInteger) override;
    Ordinal loadValue(Address address, TreatAsOrdinal) override;
    void storeValue(Address address, Ordinal value, TreatAsOrdinal) override;
    void storeValue(Address address, Integer value, TreatAsInteger) override;
    ByteInteger loadValue(Address address, TreatAsByteInteger) override;
    ByteOrdinal loadValue(Address address, TreatAsByteOrdinal) override;
    void storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal) override;
    void storeValue(Address address, ByteInteger value, TreatAsByteInteger) override;
    ShortInteger loadValue(Address address, TreatAsShortInteger) override;
    ShortOrdinal loadValue(Address address, TreatAsShortOrdinal) override;
    void storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal) override;
    void storeValue(Address address, ShortInteger value, TreatAsShortInteger) override;
    void begin() override;
private:
    bool _initialized = false;
};
using TargetBoard = MetroM4ExpressBoard;
#endif // defined ADAFRUIT_METRO_M4_EXPRESS || defined ARDUINO_METRO_M4
#endif //I960_PROTOTYPE_SIMULATOR_METROM4EXPRESS_H

//
// Created by jwscoggins on 11/26/20.
//

#ifndef I960_PROTOTYPE_SIMULATOR_TRINKET_M0_H
#define I960_PROTOTYPE_SIMULATOR_TRINKET_M0_H
#include "MemoryInterface.h"
#include "HasOnboardDotstarPixel.h"
#if defined(ARDUINO_TRINKET_M0) || defined(ADAFRUIT_TRINKET_M0)
class TrinketM0Board : public MemoryInterface, public HasOnboardDotStar<7, 8>{
public:
    using MemoryInterface::MemoryInterface;
    ~TrinketM0Board() override = default;
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
    void testFRAMArray();
private:
    bool _initialized = false;
};
using TargetBoard = TrinketM0Board;
#endif // end defined(ARDUINO_TRINKET_M0) || defined(ADAFRUIT_TRINKET_M0)
#endif //I960_PROTOTYPE_SIMULATOR_TRINKET_M0_H

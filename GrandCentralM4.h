#ifndef I960_GRAND_CENTRAL_M4_H__
#define I960_GRAND_CENTRAL_M4_H__
#include "MemoryInterface.h"
class GrandCentralM4Board : public MemoryInterface {
public:
    using MemoryInterface::MemoryInterface;
    ~GrandCentralM4Board() override = default;
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
};
using TargetBoard = GrandCentralM4Board;
#endif // end I960_GRAND_CENTRAL_M4__

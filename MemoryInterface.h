#ifndef I960_MEMORY_INTERFACE_H__
#define I960_MEMORY_INTERFACE_H__
#include "CoreTypes.h"
#include <functional>
struct TreatAsOrdinal { };
struct TreatAsInteger { };
struct TreatAsShortOrdinal { };
struct TreatAsShortInteger { };
struct TreatAsByteOrdinal { };
struct TreatAsByteInteger { };
struct MemoryInterface {
    MemoryInterface() = default;
    virtual ~MemoryInterface() = default;
    virtual Integer loadValue(Address address, TreatAsInteger) = 0;
    virtual Ordinal loadValue(Address address, TreatAsOrdinal) = 0;
    virtual void storeValue(Address address, Ordinal value, TreatAsOrdinal) = 0;
    virtual void storeValue(Address address, Integer value, TreatAsInteger) = 0;
    virtual ByteInteger loadValue(Address address, TreatAsByteInteger) = 0;
    virtual ByteOrdinal loadValue(Address address, TreatAsByteOrdinal) = 0;
    virtual void storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal) = 0;
    virtual void storeValue(Address address, ByteInteger value, TreatAsByteInteger) = 0;
    virtual ShortInteger loadValue(Address address, TreatAsShortInteger) = 0;
    virtual ShortOrdinal loadValue(Address address, TreatAsShortOrdinal) = 0;
    virtual void storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal) = 0;
    virtual void storeValue(Address address, ShortInteger value, TreatAsShortInteger) = 0;
    virtual void begin() = 0;
};
struct StubMemoryInterface : public MemoryInterface {
public:
    using MemoryInterface::MemoryInterface;
    ~StubMemoryInterface() override = default;
    Integer loadValue(Address address, TreatAsInteger) override { return 0; }
    Ordinal loadValue(Address address, TreatAsOrdinal) override { return 0; }
    void storeValue(Address address, Ordinal value, TreatAsOrdinal) override { }
    void storeValue(Address address, Integer value, TreatAsInteger) override { }
    ByteInteger loadValue(Address address, TreatAsByteInteger) override { return 0; }
    ByteOrdinal loadValue(Address address, TreatAsByteOrdinal) override { return 0; }
    void storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal) override { }
    void storeValue(Address address, ByteInteger value, TreatAsByteInteger) override { }
    ShortInteger loadValue(Address address, TreatAsShortInteger) override { return 0; }
    ShortOrdinal loadValue(Address address, TreatAsShortOrdinal) override { return 0; }
    void storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal) override { }
    void storeValue(Address address, ShortInteger value, TreatAsShortInteger) override { }
    void begin() override { }

};

#endif // end I960_MEMORY_INTERFACE_H__

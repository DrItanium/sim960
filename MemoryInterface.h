#ifndef I960_MEMORY_INTERFACE_H__
#define I960_MEMORY_INTERFACE_H__
#include "CoreTypes.h"
#include <functional>
namespace i960
{
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
    };

    struct LambdaMemoryInterface : public MemoryInterface {
        using LoadIntegerFunction = std::function<Integer(Ordinal)>;
        using LoadOrdinalFunction = std::function<Ordinal(Ordinal)>;
        using StoreIntegerFunction = std::function<void(Ordinal, Integer)>;
        using StoreOrdinalFunction = std::function<void(Ordinal, Ordinal)>;
        using LoadByteIntegerFunction = std::function<ByteInteger(Ordinal)>;
        using LoadByteOrdinalFunction = std::function<ByteOrdinal(Ordinal)>;
        using StoreByteIntegerFunction = std::function<void(Ordinal, ByteInteger)>;
        using StoreByteOrdinalFunction = std::function<void(Ordinal, ByteOrdinal)>;
        using LoadShortIntegerFunction = std::function<ShortInteger(Ordinal)>;
        using LoadShortOrdinalFunction = std::function<ShortOrdinal(Ordinal)>;
        using StoreShortIntegerFunction = std::function<void(Ordinal, ShortInteger)>;
        using StoreShortOrdinalFunction = std::function<void(Ordinal, ShortOrdinal)>;
        LambdaMemoryInterface(LoadOrdinalFunction loadOrdinal, 
                        LoadIntegerFunction loadInteger,
                        StoreOrdinalFunction storeOrdinal,
                        StoreIntegerFunction storeInteger,
                        LoadByteOrdinalFunction loadByteOrdinal,
                        LoadByteIntegerFunction loadByteInteger,
                        StoreByteOrdinalFunction storeByteOrdinal,
                        StoreByteIntegerFunction storeByteInteger,
                        LoadShortOrdinalFunction loadShortOrdinal,
                        LoadShortIntegerFunction loadShortInteger,
                        StoreShortOrdinalFunction storeShortOrdinal,
                        StoreShortIntegerFunction storeShortInteger
                        ) : 
            ldOrdinal(loadOrdinal),
            ldInteger(loadInteger),
            stOrdinal(storeOrdinal),
            stInteger(storeInteger),
            ldByteOrdinal(loadByteOrdinal),
            ldByteInteger(loadByteInteger),
            stByteOrdinal(storeByteOrdinal),
            stByteInteger(storeByteInteger),
            ldShortOrdinal(loadShortOrdinal),
            ldShortInteger(loadShortInteger),
            stShortOrdinal(storeShortOrdinal),
            stShortInteger(storeShortInteger) { }
        ~LambdaMemoryInterface() override = default;
        Integer loadValue(Address address, TreatAsInteger) override { return ldInteger(address); }
        Ordinal loadValue(Address address, TreatAsOrdinal) override { return ldOrdinal(address); }
        void storeValue(Address address, Ordinal value, TreatAsOrdinal) override { stOrdinal(address, value); }
        void storeValue(Address address, Integer value, TreatAsInteger) override { stInteger(address, value); }
        ByteInteger loadValue(Address address, TreatAsByteInteger) override { return ldByteInteger(address); }
        ByteOrdinal loadValue(Address address, TreatAsByteOrdinal) override { return ldByteOrdinal(address); }
        void storeValue(Address address, ByteOrdinal value, TreatAsByteOrdinal) override { stByteOrdinal(address, value); }
        void storeValue(Address address, ByteInteger value, TreatAsByteInteger) override { stByteInteger(address, value); }
        ShortInteger loadValue(Address address, TreatAsShortInteger) override { return ldShortInteger(address); }
        ShortOrdinal loadValue(Address address, TreatAsShortOrdinal) override { return ldShortOrdinal(address); }
        void storeValue(Address address, ShortOrdinal value, TreatAsShortOrdinal) override { stShortOrdinal(address, value); }
        void storeValue(Address address, ShortInteger value, TreatAsShortInteger) override { stShortInteger(address, value); }
        private:
            LoadOrdinalFunction ldOrdinal;
            LoadIntegerFunction ldInteger;
            StoreOrdinalFunction stOrdinal;
            StoreIntegerFunction stInteger;
            LoadByteOrdinalFunction ldByteOrdinal;
            LoadByteIntegerFunction ldByteInteger;
            StoreByteOrdinalFunction stByteOrdinal;
            StoreByteIntegerFunction stByteInteger;
            LoadShortOrdinalFunction ldShortOrdinal;
            LoadShortIntegerFunction ldShortInteger;
            StoreShortOrdinalFunction stShortOrdinal;
            StoreShortIntegerFunction stShortInteger;
    };
}

#endif // end I960_MEMORY_INTERFACE_H__

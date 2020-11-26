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
    struct TreatAsLongOrdinal { };
    struct TreatAsLongInteger { };
    struct MemoryInterface {
        MemoryInterface() = default;
        virtual ~MemoryInterface() = default;
        virtual Integer loadValue(Ordinal address, TreatAsInteger) = 0;
        virtual Ordinal loadValue(Ordinal address, TreatAsOrdinal) = 0;
        virtual void storeValue(Ordinal address, Ordinal value, TreatAsOrdinal) = 0;
        virtual void storeValue(Ordinal address, Integer value, TreatAsInteger) = 0;
        virtual ByteInteger loadValue(Ordinal address, TreatAsByteInteger) = 0;
        virtual ByteOrdinal loadValue(Ordinal address, TreatAsByteOrdinal) = 0;
        virtual void storeValue(Ordinal address, ByteOrdinal value, TreatAsByteOrdinal) = 0;
        virtual void storeValue(Ordinal address, ByteInteger value, TreatAsByteInteger) = 0;
        virtual ShortInteger loadValue(Ordinal address, TreatAsShortInteger) = 0;
        virtual ShortOrdinal loadValue(Ordinal address, TreatAsShortOrdinal) = 0;
        virtual void storeValue(Ordinal address, ShortOrdinal value, TreatAsShortOrdinal) = 0;
        virtual void storeValue(Ordinal address, ShortInteger value, TreatAsShortInteger) = 0;
        virtual LongInteger loadValue(Ordinal address, TreatAsLongInteger) = 0;
        virtual LongOrdinal loadValue(Ordinal address, TreatAsLongOrdinal) = 0;
        virtual void storeValue(Ordinal address, LongOrdinal value, TreatAsLongOrdinal) = 0;
        virtual void storeValue(Ordinal address, LongInteger value, TreatAsLongInteger) = 0;
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
        using LoadLongIntegerFunction = std::function<LongInteger(Ordinal)>;
        using LoadLongOrdinalFunction = std::function<LongOrdinal(Ordinal)>;
        using StoreLongIntegerFunction = std::function<void(Ordinal, LongInteger)>;
        using StoreLongOrdinalFunction = std::function<void(Ordinal, LongOrdinal)>;
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
                        StoreShortIntegerFunction storeShortInteger,
                        LoadLongOrdinalFunction loadLongOrdinal,
                        LoadLongIntegerFunction loadLongInteger,
                        StoreLongOrdinalFunction storeLongOrdinal,
                        StoreLongIntegerFunction storeLongInteger
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
            stShortInteger(storeShortInteger),
            ldLongOrdinal(loadLongOrdinal),
            ldLongInteger(loadLongInteger),
            stLongOrdinal(storeLongOrdinal),
            stLongInteger(storeLongInteger) { }
        ~LambdaMemoryInterface() override = default;
        Integer loadValue(Ordinal address, TreatAsInteger) override { return ldInteger(address); }
        Ordinal loadValue(Ordinal address, TreatAsOrdinal) override { return ldOrdinal(address); }
        void storeValue(Ordinal address, Ordinal value, TreatAsOrdinal) override { stOrdinal(address, value); }
        void storeValue(Ordinal address, Integer value, TreatAsInteger) override { stInteger(address, value); }
        ByteInteger loadValue(Ordinal address, TreatAsByteInteger) override { return ldByteInteger(address); }
        ByteOrdinal loadValue(Ordinal address, TreatAsByteOrdinal) override { return ldByteOrdinal(address); }
        void storeValue(Ordinal address, ByteOrdinal value, TreatAsByteOrdinal) override { stByteOrdinal(address, value); }
        void storeValue(Ordinal address, ByteInteger value, TreatAsByteInteger) override { stByteInteger(address, value); }
        ShortInteger loadValue(Ordinal address, TreatAsShortInteger) override { return ldShortInteger(address); }
        ShortOrdinal loadValue(Ordinal address, TreatAsShortOrdinal) override { return ldShortOrdinal(address); }
        void storeValue(Ordinal address, ShortOrdinal value, TreatAsShortOrdinal) override { stShortOrdinal(address, value); }
        void storeValue(Ordinal address, ShortInteger value, TreatAsShortInteger) override { stShortInteger(address, value); }
        LongInteger loadValue(Ordinal address, TreatAsLongInteger) override { return ldLongInteger(address); }
        LongOrdinal loadValue(Ordinal address, TreatAsLongOrdinal) override { return ldLongOrdinal(address); }
        void storeValue(Ordinal address, LongOrdinal value, TreatAsLongOrdinal) override { stLongOrdinal(address, value); }
        void storeValue(Ordinal address, LongInteger value, TreatAsLongInteger) override { stLongInteger(address, value); }
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
            LoadLongOrdinalFunction ldLongOrdinal;
            LoadLongIntegerFunction ldLongInteger;
            StoreLongOrdinalFunction stLongOrdinal;
            StoreLongIntegerFunction stLongInteger;
    };
}

#endif // end I960_MEMORY_INTERFACE_H__

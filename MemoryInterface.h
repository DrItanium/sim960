#ifndef I960_MEMORY_INTERFACE_H__
#define I960_MEMORY_INTERFACE_H__
#include "CoreTypes.h"
#include <functional>
namespace i960
{
    struct TreatAsOrdinal { };
    struct TreatAsInteger { };
    struct MemoryInterface {
        MemoryInterface() = default;
        virtual ~MemoryInterface() = default;
        virtual Integer loadValue(Ordinal address, TreatAsInteger) = 0;
        virtual Ordinal loadValue(Ordinal address, TreatAsOrdinal) = 0;
        virtual void storeValue(Ordinal address, Ordinal value, TreatAsOrdinal) = 0;
        virtual void storeValue(Ordinal address, Integer value, TreatAsInteger) = 0;
    };

    struct LambdaMemoryInterface : public MemoryInterface {
        using LoadIntegerFunction = std::function<Integer(Ordinal)>;
        using LoadOrdinalFunction = std::function<Ordinal(Ordinal)>;
        using StoreIntegerFunction = std::function<void(Ordinal, Integer)>;
        using StoreOrdinalFunction = std::function<void(Ordinal, Ordinal)>;
        MemoryInterface(LoadOrdinalFunction ldOrdinal, 
                        LoadIntegerFunction ldInteger,
                        StoreOrdinalFunction stOrdinal,
                        StoreIntegerFunction stInteger) : lof(ldOrdinal), lif(ldInteger), sof(stOrdinal), sif(stInteger) { }
        ~LambdaMemoryInterface() override = default;
        Integer loadValue(Ordinal address, TreatAsInteger) override { return lif(address); }
        Ordinal loadValue(Ordinal address, TreatAsOrdinal) override { return lof(address); }
        void storeValue(Ordinal address, Ordinal value, TreatAsOrdinal) override { sof(address, value); }
        void storeValue(Ordinal address, Integer value, TreatAsInteger) override { sif(address, value); }
        private:
            LoadOrdinalFunction lof;
            LoadIntegerFunction lif;
            StoreOrdinalFunction sof;
            StoreIntegerFunction sif;
    };
}

#endif // end I960_MEMORY_INTERFACE_H__

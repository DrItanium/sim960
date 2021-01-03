#ifndef I960_CORE_TYPES_H__
#define I960_CORE_TYPES_H__
#include <cstdint>
using Ordinal = std::uint32_t;
using Integer = std::int32_t;
using ShortOrdinal = std::uint16_t;
using ShortInteger = std::int16_t;
using LongOrdinal = std::uint64_t;
using LongInteger = std::int64_t;
using ByteOrdinal = std::uint8_t;
using ByteInteger = std::int8_t;
using Address = Ordinal;
using Real = float;
using LongReal = double;
using ExtendedReal = long double;
template<typename T>
struct TreatAs final { using ReturnType = T; };
using TreatAsOrdinal = TreatAs<Ordinal>;
using TreatAsInteger = TreatAs<Integer>;
using TreatAsByteOrdinal = TreatAs<ByteOrdinal>;
using TreatAsByteInteger = TreatAs<ByteInteger>;
using TreatAsShortInteger = TreatAs<ShortInteger>;
using TreatAsShortOrdinal = TreatAs<ShortOrdinal>;
using TreatAsLongOrdinal = TreatAs<LongOrdinal>;
using TreatAsLongInteger = TreatAs<LongInteger>;
using TreatAsReal = TreatAs<Real>;
using TreatAsLongReal = TreatAs<LongReal>;
using TreatAsExtendedReal = TreatAs<ExtendedReal>;
#endif // end I960_CORE_TYPES_H__

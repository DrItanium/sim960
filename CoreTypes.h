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
struct TreatAsOrdinal final { using ReturnType = Ordinal; };
struct TreatAsInteger final { using ReturnType = Integer; };
struct TreatAsByteOrdinal final { using ReturnType = ByteOrdinal; };
struct TreatAsByteInteger final { using ReturnType = ByteInteger; };
struct TreatAsShortOrdinal final { using ReturnType = ShortOrdinal; };
struct TreatAsShortInteger final { using ReturnType = ShortInteger; };
struct TreatAsLongOrdinal final { using ReturnType = LongOrdinal; };
struct TreatAsLongInteger final { using ReturnType = LongInteger; };
#endif // end I960_CORE_TYPES_H__

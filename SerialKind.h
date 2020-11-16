#ifndef I960_SERIAL_KIND_H__
#define I960_SERIAL_KIND_H__

struct IsHardwareBasedSerial final { constexpr IsHardwareBasedSerial() = default; };
struct IsSoftwareBasedSerial final { constexpr IsSoftwareBasedSerial() = default; };

#endif // end I960_SERIAL_KIND_H__

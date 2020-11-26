#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include "CoreTypes.h"
#include "TargetPlatform.h"

namespace i960
{
  class Register {
    public:
    constexpr Register() : ordValue(0) { }
    constexpr Ordinal getOrdinalValue() const noexcept { return ordValue; }
    constexpr Integer getIntegerValue() const noexcept { return intValue; }
    constexpr auto getShortOrdinal(bool upper = false) const noexcept { return shortOrds[upper ? 1 : 0 ]; }
    constexpr auto getShortInteger(bool upper = false) const noexcept { return shortInts[upper ? 1 : 0 ]; }
    constexpr auto getByteOrdinal(int index = 0) const noexcept { return byteOrds[index & 0b11]; }
    constexpr auto getByteInteger(int index = 0) const noexcept { return byteInts[index & 0b11]; }
    void setOrdinal(Ordinal value) noexcept { ordValue = value; }
    void setInteger(Integer value) noexcept { intValue = value; }
    void setShortOrdinal(ShortOrdinal value, bool upper = false) noexcept { shortOrds[upper ? 1 : 0] = value; }
    void setShortInteger(ShortInteger value, bool upper = false) noexcept { shortInts[upper ? 1 : 0] = value; }
    void setByteOrdinal(ByteOrdinal value, int index = 0) noexcept { byteOrds[index & 0b11] = value; }
    void setByteInteger(ByteInteger value, int index = 0) noexcept { byteInts[index & 0b11] = value; }
    void increment() noexcept { ++ordValue; }
    void decrement() noexcept { --ordValue; }
    private:
      union {
        Ordinal ordValue;
        Integer intValue;
        ShortOrdinal shortOrds[2];
        ShortInteger shortInts[2];
        ByteOrdinal byteOrds[4];
        ByteInteger byteInts[4];
      };
  };
  using RegisterFile = std::array<Register, 16>;
  class Core {
    public:
      using Pins = TargetBoardPinout;
    public:
      void cycle();
    private:
      Ordinal retrieveInstruction();
      Register& getRegister(int index) noexcept;
      const Register& getRegister(int index) const noexcept;
      void moveRegisterContents(int from, int to) noexcept;
      
    private:
      RegisterFile globals, locals;
      Register ip; // always start at address zero
  };
}
#endif // end I960_CORE_H__

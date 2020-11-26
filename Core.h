#ifndef I960_CORE_H__
#define I960_CORE_H__
#include <array>
#include "CoreTypes.h"
#include "TargetPlatform.h"
#include "MemoryInterface.h"

namespace i960
{
  class Register {
    public:
    constexpr Register() : ordValue(0) { }
    constexpr Ordinal getOrdinal() const noexcept { return ordValue; }
    constexpr Integer getInteger() const noexcept { return intValue; }
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
    explicit operator Ordinal() const noexcept { return getOrdinal(); }
    explicit operator Integer() const noexcept { return getInteger(); }
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
  class LongRegister {
      public:
          LongRegister(Register& lower, Register& upper) noexcept : _lower(lower), _upper(upper) { }
          void setOrdinal(LongOrdinal value) noexcept { 
              _lower.setOrdinal(static_cast<Ordinal>(value));
              _upper.setOrdinal(static_cast<Ordinal>(value >> 32));
          }
          void setInteger(LongInteger value) noexcept { 
              _lower.setInteger(static_cast<Integer>(value));
              _upper.setInteger(static_cast<Integer>(value >> 32));
          }
          constexpr LongOrdinal getOrdinal() const noexcept {
              auto lword = static_cast<LongOrdinal>(_lower.getOrdinal());
              auto uword = static_cast<LongOrdinal>(_upper.getOrdinal()) << 32;
              return lword | uword;
          }
          constexpr LongInteger getInteger() const noexcept {
              auto lword = static_cast<LongInteger>(_lower.getInteger());
              auto uword = static_cast<LongInteger>(_upper.getInteger()) << 32;
              return lword | uword;
          }
          explicit operator LongOrdinal() const noexcept { return getOrdinal(); }
          explicit operator LongInteger() const noexcept { return getInteger(); }
      private:
          Register& _lower;
          Register& _upper;
  };

  using RegisterFile = std::array<Register, 16>;
  class Core {
    public:
      using Pins = TargetBoardPinout;
    public:
      Core(MemoryInterface& mi);
      void cycle();
    private:
      Register& getRegister(int index) noexcept;
      const Register& getRegister(int index) const noexcept;
      void moveRegisterContents(int from, int to) noexcept;
      void saveLocals() noexcept;
      void restoreLocals() noexcept;
      Ordinal loadOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsOrdinal{}); }
      void storeOrdinal (Address address, Ordinal value) noexcept { memoryController.storeValue(address, value, TreatAsOrdinal{}); }
      Integer loadInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsInteger{}); }
      void storeInteger (Address address, Integer value) noexcept { memoryController.storeValue(address, value, TreatAsInteger{}); }

      ByteOrdinal loadByteOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsByteOrdinal{}); }
      void storeByteOrdinal (Address address, ByteOrdinal value) noexcept { memoryController.storeValue(address, value, TreatAsByteOrdinal{}); }
      ByteInteger loadByteInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsByteInteger{}); }
      void storeByteInteger (Address address, ByteInteger value) noexcept { memoryController.storeValue(address, value, TreatAsByteInteger{}); }

      LongOrdinal loadLongOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsLongOrdinal{}); }
      void storeLongOrdinal (Address address, LongOrdinal value) noexcept { memoryController.storeValue(address, value, TreatAsLongOrdinal{}); }
      LongInteger loadLongInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsLongInteger{}); }
      void storeLongInteger (Address address, LongInteger value) noexcept { memoryController.storeValue(address, value, TreatAsLongInteger{}); }

      ShortOrdinal loadShortOrdinal(Address address) noexcept { return memoryController.loadValue(address, TreatAsShortOrdinal{}); }
      void storeShortOrdinal (Address address, ShortOrdinal value) noexcept { memoryController.storeValue(address, value, TreatAsShortOrdinal{}); }
      ShortInteger loadShortInteger(Address address) noexcept { return memoryController.loadValue(address, TreatAsShortInteger{}); }
      void storeShortInteger (Address address, ShortInteger value) noexcept { memoryController.storeValue(address, value, TreatAsShortInteger{}); }


    private:
      MemoryInterface& memoryController;
      RegisterFile globals, locals;
      Register ip; // always start at address zero
  };
}
#endif // end I960_CORE_H__

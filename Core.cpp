#include "Core.h"

namespace i960 {
void
Core::cycle()
{
  
}
Register&
Core::getRegister(int index) noexcept {
  if (auto offset = index & 0b1111, maskedValue = index & 0b10000; maskedValue != 0) {
    return locals[offset];
  } else {
    return globals[offset];
  }
}

const Register&
Core::getRegister(int index) const noexcept {
  if (auto offset = index & 0b1111, maskedValue = index & 0b10000; maskedValue != 0) {
    return locals[offset];
  } else {
    return globals[offset];
  }
}

void
Core::moveRegisterContents(int from, int to) noexcept {
  getRegister(to).setOrdinal(static_cast<Ordinal>(getRegister(from)));
}
Core::Core(MemoryInterface& mi) : memoryController(mi) { }

void
Core::saveLocals() noexcept
{
    // okay, we have to save all of the registers to the stack or the on board
    // register cache (however, I'm not implementing that yet)
}
void
Core::restoreLocals() noexcept
{
    // restore the local register frame, generally done when you return from a
    // previous function
}

void
Core::storeLongRegister(Address address, int baseIndex) noexcept {
    /// @todo error out if the long register is not even
    storeOrdinal(address, static_cast<Ordinal>(getRegister(baseIndex)));
    storeOrdinal(address+4, static_cast<Ordinal>(getRegister(baseIndex + 1)));
}

void
Core::loadLongRegister(Address address, int baseIndex) noexcept {
    getRegister(baseIndex).setOrdinal(loadOrdinal(address));
    getRegister(baseIndex+1).setOrdinal(loadOrdinal(address+4));
}

void
Core::storeTripleRegister(Address address, int baseIndex) noexcept {
    /// @todo error out if the long register is not even
    storeOrdinal(address, static_cast<Ordinal>(getRegister(baseIndex)));
    storeOrdinal(address+4, static_cast<Ordinal>(getRegister(baseIndex + 1)));
    storeOrdinal(address+8, static_cast<Ordinal>(getRegister(baseIndex + 2)));
}

void
Core::loadTripleRegister(Address address, int baseIndex) noexcept {
    getRegister(baseIndex).setOrdinal(loadOrdinal(address));
    getRegister(baseIndex+1).setOrdinal(loadOrdinal(address+4));
    getRegister(baseIndex+2).setOrdinal(loadOrdinal(address+8));
}

} // end namespace i960

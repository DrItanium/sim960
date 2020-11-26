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
} // end namespace i960

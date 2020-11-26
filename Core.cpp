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
  getRegister(to).setOrdinal(getRegister(from).getOrdinalValue());
}
}

#ifndef I960_GRAND_CENTRAL_M4_H__
#define I960_GRAND_CENTRAL_M4_H__
#include "SerialKind.h"
namespace GrandCentralM4
{
enum class Pinout : uint8_t {
    // i960Sx Pinout
  Lock = 22, // active low, open collector, bi directional
  INT0, // active low input
  INT1, // active low 
  INT2,
  INT3,
  Ready,
  Hold,
  ALE,
  AS,
  BLAST,
  DT_R,
  DEN,
  W_R,
  HLDA,
  BE0,
  BE1,
  BA1,
  BA2,
  BA3,
  // continue here
  Last,
  First = Lock,
};
}
using TargetBoardPinout = GrandCentralM4::Pinout;
using SerialKind = IsSoftwareBasedSerial;
#endif // end I960_GRAND_CENTRAL_M4__

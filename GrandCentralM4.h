#ifndef I960_GRAND_CENTRAL_M4__
#define I960_GRAND_CENTRAL_M4__

enum class GrandCentralM4Pinout : uint8_t {
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
using TargetBoardPinout = GrandCentralM4Pinout;
#endif // end I960_GRAND_CENTRAL_M4__

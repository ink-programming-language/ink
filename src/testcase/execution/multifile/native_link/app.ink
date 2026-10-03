import provider;

import "C" func inkNativeBoundaryValue(): i32;

// Compile-time and runtime native references both resolve the loaded provider's export body.
comptime var Saved: i32 = inkNativeBoundaryValue();

func main(): i32
{
  return Saved + inkNativeBoundaryValue();
}

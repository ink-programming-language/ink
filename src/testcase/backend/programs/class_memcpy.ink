class Box
{
  field Value: i32;

  func __init__(InitialValue: i32): void
  {
    this.Value = InitialValue;
  }
};

import "C" func memcpy(Destination: *Box, Source: *Box, Count: u64): *Box;

func main(): i32
{
  var ObjectB = Box(42);
  var ObjectA = Box(43);
  var ExistingAddress = &ObjectB.Value;
  memcpy(&ObjectB, &ObjectA, 4);
  ObjectA.Value = 99;
  if (*ExistingAddress != 43)
  {
    return 1;
  }
  return ObjectB.Value;
}

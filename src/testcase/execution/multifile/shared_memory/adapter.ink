// The adapter forwards the original address to two functions in the storage provider.
from memory import increase;
from memory import alias;

// Writes through the original pointer and its returned alias must accumulate in one caller-owned cell.
func update(Address: *i32): *i32
{
  increase(Address, 19);
  var SameAddress = alias(Address);
  increase(SameAddress, 23);
  return SameAddress;
}

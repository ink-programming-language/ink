// This import points back to the first module, closing the cross-file call cycle.
from first import countFirst;

// The cycle terminates at zero while preserving an independent argument in every activation.
func countSecond(Depth: i32): i32
{
  if (Depth <= 0)
  {
    return 0;
  }
  return countFirst(Depth + -1) + 1;
}

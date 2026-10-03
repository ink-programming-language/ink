// The next recursive activation belongs to a different source file.
from second import countSecond;

// Returning from the second module resumes this frame's pending addition.
func countFirst(Depth: i32): i32
{
  if (Depth <= 0)
  {
    return 0;
  }
  return countSecond(Depth + -1) + 1;
}

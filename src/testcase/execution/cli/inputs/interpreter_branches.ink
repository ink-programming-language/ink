// Resolving this nonexistent native symbol would expose an incorrectly executed unselected branch.
import "C" func InkMissingBranchProcessSymbol71e4935b(): i32;

func missing(): i32
{
  return InkMissingBranchProcessSymbol71e4935b();
}

// Both arms initialize the same local before the shared continuation reads it.
func choose(Flag: bool): i32
{
  var Result: i32;
  if (Flag)
  {
    Result = 19;
  }
  else
  {
    Result = 23;
  }
  return Result;
}

func main(): i32
{
  return choose(true);
}

func alternate(): i32
{
  return choose(false);
}

// The valid wrapper has a body, but its native call must remain unexecuted.
func skipped(): i32
{
  if (false)
  {
    return missing();
  }
  else
  {
    return 17;
  }
}

comptime func staticChoose(Flag: bool): i32
{
  if (Flag)
  {
    return 29;
  }
  else
  {
    return 31;
  }
}

func compiled(): i32
{
  return comptime staticChoose(false);
}

// Check preconditions, repeated tests, transfers and nested runtime while loops.
import "C" func puts(Text: *u8): i32;

func sum(Limit: i32): i32
{
  var I = 0;
  var Sum = 0;
  while (I < Limit)
  {
    ++I;
    Sum += I;
  }
  return Sum;
}

// Continue rechecks the condition; break does not execute another body or condition.
func transfers(): i32
{
  var I = 0;
  var Sum = 0;
  while (true)
  {
    I++;
    if (I == 2) continue;
    if (I == 5) break;
    Sum += I;
  }
  return Sum;
}

// Inner transfers leave the outer counter and its continuation intact.
func nested(): i32
{
  var I = 0;
  var Sum = 0;
  while (I < 3)
  {
    I++;
    var J = 0;
    while (J < 5)
    {
      J++;
      if (J == 2) continue;
      if (J == 4) break;
      Sum += I + J;
    }
  }
  return Sum;
}

func earlyReturn(Limit: i32): i32
{
  var I = 0;
  while (I < Limit)
  {
    if (I == 3) return 42;
    I++;
  }
  return 7;
}

func bareBody(): i32
{
  var I = 4;
  while (I > 0) --I;
  return I;
}

func check(Actual: i32, Expected: i32): bool
{
  if (Actual == Expected)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  puts("While.zero");
  if (!check(sum(0), 0)) return 1;
  puts("While.one");
  if (!check(sum(1), 1)) return 1;
  puts("While.many");
  if (!check(sum(8), 36)) return 1;
  puts("While.break_continue");
  if (!check(transfers(), 8)) return 1;
  puts("While.nested");
  if (!check(nested(), 24)) return 1;
  puts("While.early_return");
  if (!check(earlyReturn(10), 42)) return 1;
  puts("While.fallthrough");
  if (!check(earlyReturn(2), 7)) return 1;
  puts("While.bare_body");
  if (!check(bareBody(), 0)) return 1;
  return 0;
}

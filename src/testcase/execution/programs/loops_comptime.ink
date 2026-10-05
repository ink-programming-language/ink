// Ordinary loop IR also executes in comptime calls, while static loops expand runtime statements.
import "C" func puts(Text: *u8): i32;

comptime func calculate(): i32
{
  var Sum = 0;
  for (var I = 0; I < 4; I++)
  {
    if (I == 1) continue;
    Sum += I;
  }
  while (Sum < 10) Sum++;
  for (Item in [10, 22]) Sum += Item;
  return Sum;
}

comptime var SelectedSum = 0;
comptime
{
  for (Item in [1, 2, 3, 4, 5])
  {
    if (Item == 2) continue;
    if (Item == 5) break;
    SelectedSum += Item;
  }
}

func expandedArray(): i32
{
  var Sum = 0;
  comptime for ([A, B] in [[1, 2], [3, 4]]) Sum += A + B;
  return Sum;
}

func runtimeInsideStatic(): i32
{
  var Sum = 0;
  comptime for (Item in [1, 2, 3])
  {
    for (var I = 0; I < 4; I++)
    {
      if (I == 2) break;
      Sum += Item;
    }
  }
  return Sum;
}

func staticInsideRuntime(): i32
{
  var Sum = 0;
  var I = 0;
  while (I < 3)
  {
    I++;
    comptime for (Item in [1, 2, 3])
    {
      comptime if (Item == 2) continue;
      Sum += Item;
    }
  }
  return Sum;
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
  puts("LoopComptime.lowered_call");
  if (!check(comptime calculate(), 42)) return 1;
  puts("LoopComptime.array_transfers");
  if (!check(SelectedSum, 8)) return 1;
  puts("LoopComptime.array_expansion");
  if (!check(expandedArray(), 10)) return 1;
  puts("LoopComptime.runtime_inside_static");
  if (!check(runtimeInsideStatic(), 12)) return 1;
  puts("LoopComptime.static_inside_runtime");
  if (!check(staticInsideRuntime(), 12)) return 1;
  return 0;
}

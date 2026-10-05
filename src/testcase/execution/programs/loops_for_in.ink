// Check array snapshots, mutable element copies, empty arrays and recursive binding patterns.
import "C" func puts(Text: *u8): i32;

func snapshot(): i32
{
  var Values = [1, 2, 3];
  var Sum = 0;
  for (Item in Values)
  {
    Values[1] = 99;
    Item += 10;
    Sum += Item;
  }
  var Item = 6;
  return Sum + Item;
}

func empty(): i32
{
  var Values: [i32; 0] = [];
  var Result = 42;
  for (Item in Values) Result = Item;
  return Result;
}

func transfers(): i32
{
  var Sum = 0;
  for (Item in [1, 2, 3, 4, 5])
  {
    if (Item == 2) continue;
    if (Item == 5) break;
    Sum += Item;
  }
  return Sum;
}

func nested(): i32
{
  var Sum = 0;
  for (Row in [[1, 2], [3, 4]])
  {
    for (Item in Row) Sum += Item;
  }
  return Sum;
}

func patterns(): i32
{
  var Sum = 0;
  for ([Head, Tail...] in [[1, 2, 3], [4, 5, 6]])
  {
    Sum += Head;
    for (Item in Tail) Sum += Item;
  }
  return Sum;
}

func recursivePatterns(): i32
{
  var Sum = 0;
  for ([[A, _], [B, _]] in [[[1, 2], [3, 4]]]) Sum += A + B;
  for ([Head, _...] in [[5, 99]]) Sum += Head;
  for ([Head, Tail...] in [[6]])
  {
    Sum += Head;
    for (_ in Tail) Sum++;
  }
  return Sum;
}

func wildcards(): i32
{
  var Count = 0;
  for (_ in [7, 8, 9]) Count++;
  for (Flag in [false, true, true]) if (Flag) Count++;
  return Count;
}

func earlyReturn(): i32
{
  for (Item in [2, 42, 3]) if (Item == 42) return Item;
  return 0;
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
  puts("ForIn.snapshot_and_copy");
  if (!check(snapshot(), 42)) return 1;
  puts("ForIn.empty");
  if (!check(empty(), 42)) return 1;
  puts("ForIn.break_continue");
  if (!check(transfers(), 8)) return 1;
  puts("ForIn.nested");
  if (!check(nested(), 10)) return 1;
  puts("ForIn.array_patterns");
  if (!check(patterns(), 21)) return 1;
  puts("ForIn.recursive_patterns");
  if (!check(recursivePatterns(), 15)) return 1;
  puts("ForIn.wildcards_and_bools");
  if (!check(wildcards(), 5)) return 1;
  puts("ForIn.early_return");
  if (!check(earlyReturn(), 42)) return 1;
  return 0;
}

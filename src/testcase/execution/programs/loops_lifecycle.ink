// Every exit path cleans exactly the objects whose lifetimes end, in reverse construction order.
import "C" func puts(Text: *u8): i32;

class Watch
{
  field Count: *i32;
  field Tag: i32;

  func __init__(Count: *i32, Tag: i32): void
  {
    this.Count = Count;
    this.Tag = Tag;
  }

  func __del__(): void
  {
    (*this.Count)++;
    if (this.Tag == 1) puts("body");
    else if (this.Tag == 2) puts("inner");
    else puts("header");
  }

  func yes(): bool
  {
    return true;
  }
};

func forCleanup(): i32
{
  var Count = 0;
  var I = 0;
  for (var Header = Watch(&Count, 3); I < 4; I++)
  {
    var Body = Watch(&Count, 1);
    if (I == 0) continue;
    var Inner = Watch(&Count, 2);
    if (I == 2) break;
  }
  return Count;
}

func returnFromLoop(Count: *i32): void
{
  for (var Header = Watch(Count, 3); true;)
  {
    var Body = Watch(Count, 1);
    return;
  }
}

func returnCleanup(): i32
{
  var Count = 0;
  returnFromLoop(&Count);
  return Count;
}

func iterableCleanup(): i32
{
  var Count = 0;
  for (Item in [Watch(&Count, 1), Watch(&Count, 2)]) break;
  return Count;
}

func conditionCleanup(): i32
{
  var Count = 0;
  var I = 0;
  while (I < 2 && Watch(&Count, 1).yes())
  {
    I++;
    continue;
  }
  return Count;
}

func stepCleanup(): i32
{
  var Count = 0;
  for (var I = 0; I < 3; I++, Watch(&Count, 2))
  {
    if (I == 0) continue;
    if (I == 2) break;
  }
  return Count;
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
  puts("LoopLifecycle.for_cleanup");
  if (!check(forCleanup(), 6)) return 1;
  puts("LoopLifecycle.return_cleanup");
  if (!check(returnCleanup(), 2)) return 1;
  puts("LoopLifecycle.iterable_cleanup");
  if (!check(iterableCleanup(), 3)) return 1;
  puts("LoopLifecycle.condition_cleanup");
  if (!check(conditionCleanup(), 2)) return 1;
  puts("LoopLifecycle.step_cleanup");
  if (!check(stepCleanup(), 2)) return 1;
  return 0;
}

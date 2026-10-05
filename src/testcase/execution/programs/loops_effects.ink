// Exact stdout checks evaluation order independently from the final counter values.
import "C" func puts(Text: *u8): i32;

func condition(Calls: *i32): bool
{
  puts("condition");
  (*Calls)++;
  return *Calls < 3;
}

func initialize(): i32
{
  puts("init");
  return 0;
}

func step(): void
{
  puts("step");
}

func make(Calls: *i32): [i32; 3]
{
  puts("iterable");
  (*Calls)++;
  return [1, 2, 3];
}

func whileEffects(): i32
{
  var Calls = 0;
  while (condition(&Calls)) puts("body");
  return Calls;
}

func forEffects(): i32
{
  var Sum = 0;
  for (var I = initialize(); I < 4; I++, step())
  {
    puts("body");
    if (I == 0) continue;
    Sum += I;
    if (I == 2) break;
  }
  return Sum;
}

func iterableEffects(): i32
{
  var Calls = 0;
  var Sum = 0;
  for (Item in make(&Calls))
  {
    puts("element");
    Sum += Item;
  }
  return Sum + Calls;
}

func skippedEffects(): i32
{
  var Calls = 0;
  while (false && condition(&Calls)) puts("unexpected");
  for (; false && condition(&Calls); step()) puts("unexpected");
  return Calls;
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
  puts("LoopEffects.while_condition");
  if (!check(whileEffects(), 3)) return 1;
  puts("LoopEffects.for_order");
  if (!check(forEffects(), 3)) return 1;
  puts("LoopEffects.iterable_once");
  if (!check(iterableEffects(), 7)) return 1;
  puts("LoopEffects.short_circuit");
  if (!check(skippedEffects(), 0)) return 1;
  return 0;
}

// Check classic headers, ordered steps, variable scopes and nearest-loop transfers.
import "C" func puts(Text: *u8): i32;

func sum(Limit: i32): i32
{
  var Sum = 0;
  for (var I = 0; I < Limit; I++) Sum += I;
  return Sum;
}

func descending(): i32
{
  var Sum = 0;
  for (var I = 4; I > 0; --I) Sum += I;
  return Sum;
}

// Every header part is optional; an unconditional loop may initialize a local before break.
func omittedHeaders(): i32
{
  var I = 0;
  for (; I < 3;) I++;
  for (;;)
  {
    I++;
    if (I == 5) break;
  }
  for (I = 40;; I++) if (I == 42) break;
  var Result: i32;
  for (;;)
  {
    Result = I;
    break;
  }
  return Result;
}

// Initializers and steps run left to right; continue executes steps and break skips them.
func steps(): i32
{
  var I = 9;
  var Steps = 0;
  var Sum = 0;
  for (I = 0, Steps = I + 10; I < 5; I++, Steps += I)
  {
    if (I == 1) continue;
    if (I == 4) break;
    Sum += I;
  }
  return Steps + Sum;
}

func scopedLocals(): i32
{
  var I = 42;
  for (var I = 0; I < 3; I++) var Local = I;
  var Local = 0;
  return I + Local;
}

func nested(): i32
{
  var Sum = 0;
  for (var I = 0; I < 3; I++)
  {
    for (var J = 0; J < 4; J++)
    {
      if (J == 1) continue;
      if (J == 3) break;
      Sum += I + J;
    }
  }
  return Sum;
}

func earlyReturn(): i32
{
  for (;;) return 42;
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
  puts("For.zero");
  if (!check(sum(0), 0)) return 1;
  puts("For.one");
  if (!check(sum(1), 0)) return 1;
  puts("For.many");
  if (!check(sum(6), 15)) return 1;
  puts("For.descending");
  if (!check(descending(), 10)) return 1;
  puts("For.omitted_headers");
  if (!check(omittedHeaders(), 42)) return 1;
  puts("For.step_order");
  if (!check(steps(), 25)) return 1;
  puts("For.scope");
  if (!check(scopedLocals(), 42)) return 1;
  puts("For.nested");
  if (!check(nested(), 12)) return 1;
  puts("For.early_return");
  if (!check(earlyReturn(), 42)) return 1;
  return 0;
}

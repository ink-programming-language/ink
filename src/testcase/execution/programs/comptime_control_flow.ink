// Verify compile-time state, selected branches and static loops through runtime result assertions.
import "C" func puts(Text: *u8): i32;

comptime var Snapshot: i32 = 2;

func beforeWrite(): i32
{
  return comptime Snapshot;
}

comptime
{
  Snapshot = 9;
}

func afterWrite(): i32
{
  return comptime Snapshot;
}

// Continue still performs the for step; break stops before any later iteration contributes.
comptime var ForSum: i32 = 0;
comptime
{
  for (var I: i32 = 0; I < 6; I++)
  {
    if (I == 1) continue;
    if (I == 4) break;
    ForSum = ForSum + I;
  }
}

// A while loop rechecks the mutated condition after continue and stops immediately at break.
comptime var WhileSum: i32 = 0;
comptime
{
  var I: i32 = 0;
  while (I < 6)
  {
    I++;
    if (I == 2) continue;
    if (I == 5) break;
    WhileSum = WhileSum + I;
  }
}

// Inner locals are recreated on every outer iteration and both loop variables remain independent.
comptime var NestedSum: i32 = 0;
comptime
{
  for (var I: i32 = 0; I < 3; I++)
  {
    for (var J: i32 = 0; J < 2; J++)
    {
      NestedSum = NestedSum + I + J;
    }
  }
}

// Zero iterations must neither evaluate the body nor change the accumulator.
comptime var ZeroIterations: i32 = 17;
comptime
{
  while (false)
  {
    ZeroIterations = 99;
  }
  for (var I: i32 = 0; I < 0; I++)
  {
    ZeroIterations = 100;
  }
}

// Static for expansion emits runtime branches that still depend on the actual call argument.
func unrolledBranches(Flag: bool): i32
{
  var Result: i32 = 0;
  comptime for (var I: i32 = 0; I < 4; I++)
  {
    if (Flag) Result = Result + 1;
    else Result = Result + 2;
  }
  return Result;
}

// Static while expansion honors continue and break while emitting the surviving runtime additions.
func unrolledWhile(): i32
{
  var Result: i32 = 0;
  comptime var I: i32 = 0;
  comptime while (I < 5)
  {
    comptime { I++; }
    comptime if (I == 2) continue;
    comptime if (I == 4) break;
    Result = Result + comptime I;
  }
  return Result;
}

// The unselected comptime branch is not analyzed, so its unknown name cannot fail the program.
func selectedBranch(): i32
{
  comptime if (false) return UnknownName;
  else return 23;
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
  puts("Static.before_write");
  if (!check(beforeWrite(), 2)) return 1;
  puts("Static.after_write");
  if (!check(afterWrite(), 9)) return 1;

  puts("Static.for_continue_break");
  if (!check(comptime ForSum, 5)) return 1;
  puts("Static.while_continue_break");
  if (!check(comptime WhileSum, 8)) return 1;
  puts("Static.nested_loops");
  if (!check(comptime NestedSum, 9)) return 1;
  puts("Static.zero_iterations");
  if (!check(comptime ZeroIterations, 17)) return 1;

  puts("Static.unrolled_true");
  if (!check(unrolledBranches(true), 4)) return 1;
  puts("Static.unrolled_false");
  if (!check(unrolledBranches(false), 8)) return 1;
  puts("Static.unrolled_while");
  if (!check(unrolledWhile(), 4)) return 1;
  puts("Static.selected_branch");
  if (!check(selectedBranch(), 23)) return 1;
  return 0;
}

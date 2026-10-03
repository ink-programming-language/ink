// Check both selected and unselected paths, initialization merges, lexical scopes and observable branch effects.
import "C" func puts(Text: *u8): i32;

// Executing this wrapper would try to resolve a deliberately nonexistent native symbol.
import "C" func InkMissingControlFlowSymbol71e4935b(): i32;

func missing(): i32
{
  return InkMissingControlFlowSymbol71e4935b();
}

// Each arm initializes the same local before the common continuation reads it.
func select(Flag: bool): i32
{
  var Result: i32;
  if (Flag)
  {
    Result = 11;
  }
  else
  {
    Result = 22;
  }
  return Result + 1;
}

// The else-if chain selects exactly one of negative, zero and positive values.
func classify(Value: i32): i32
{
  if (Value < 0) return 1;
  else if (Value == 0) return 2;
  else return 3;
}

// All four nested paths initialize the result before their control-flow merge.
func nested(First: bool, Second: bool): i32
{
  var Result: i32;
  if (First)
  {
    if (Second) Result = 10;
    else Result = 20;
  }
  else
  {
    if (Second) Result = 30;
    else Result = 40;
  }
  return Result;
}

// A returning arm need not initialize a local that only the surviving arm reads.
func initializeAfterReturn(Flag: bool): i32
{
  var Result: i32;
  if (Flag) return 7;
  else Result = 9;
  return Result;
}

// Branch-local shadowing must neither replace the outer local nor escape into its sibling.
func shadow(Flag: bool): i32
{
  var Value: i32 = 5;
  var Result: i32 = 0;
  if (Flag)
  {
    var Value: i32 = 10;
    Result = Value;
  }
  else
  {
    var Value: i32 = 20;
    Result = Value;
  }
  return Value + Result;
}

// An empty true arm and a missing else both reach the next statement.
func emptyFallthrough(Flag: bool): i32
{
  var Result: i32 = 4;
  if (Flag)
  {
  }
  if (!Flag) Result = Result + 3;
  return Result;
}

func condition(Flag: bool): bool
{
  puts("condition");
  return Flag;
}

// Exact stdout asserts the condition runs once, only the chosen arm runs and the merge runs once.
func effects(Flag: bool): i32
{
  var Result: i32;
  if (condition(Flag))
  {
    puts("then");
    Result = 1;
  }
  else
  {
    puts("else");
    Result = 2;
  }
  puts("merge");
  return Result;
}

// An unreachable branch must never execute the wrapper that resolves the missing native symbol.
func skipMissing(Flag: bool): i32
{
  if (Flag)
  {
    if (false) return missing();
    return 31;
  }
  else
  {
    if (true) return 32;
    return missing();
  }
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
  puts("Control.select_true");
  if (!check(select(true), 12)) return 1;
  puts("Control.select_false");
  if (!check(select(false), 23)) return 1;

  puts("Control.negative");
  if (!check(classify(-1), 1)) return 1;
  puts("Control.zero");
  if (!check(classify(0), 2)) return 1;
  puts("Control.positive");
  if (!check(classify(1), 3)) return 1;

  puts("Control.nested_true_true");
  if (!check(nested(true, true), 10)) return 1;
  puts("Control.nested_true_false");
  if (!check(nested(true, false), 20)) return 1;
  puts("Control.nested_false_true");
  if (!check(nested(false, true), 30)) return 1;
  puts("Control.nested_false_false");
  if (!check(nested(false, false), 40)) return 1;

  puts("Control.returning_arm");
  if (!check(initializeAfterReturn(true), 7)) return 1;
  puts("Control.surviving_arm");
  if (!check(initializeAfterReturn(false), 9)) return 1;

  puts("Control.shadow_true");
  if (!check(shadow(true), 15)) return 1;
  puts("Control.shadow_false");
  if (!check(shadow(false), 25)) return 1;

  puts("Control.empty_true");
  if (!check(emptyFallthrough(true), 4)) return 1;
  puts("Control.empty_false");
  if (!check(emptyFallthrough(false), 7)) return 1;

  puts("Control.effects_true");
  if (!check(effects(true), 1)) return 1;
  puts("Control.effects_false");
  if (!check(effects(false), 2)) return 1;

  puts("Control.skip_missing_true");
  if (!check(skipMissing(true), 31)) return 1;
  puts("Control.skip_missing_false");
  if (!check(skipMissing(false), 32)) return 1;
  return 0;
}

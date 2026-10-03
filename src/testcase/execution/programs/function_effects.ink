// Exact stdout records call ordering and detects duplicated or skipped side effects.
import "C" func puts(Text: *u8): i32;

// The first operand reports when its call executes.
func first(Value: i32): i32
{
  puts("first");
  return Value;
}

// The second operand reports when its call executes.
func second(Value: i32): i32
{
  puts("second");
  return Value;
}

// The third operand reports when its call executes.
func third(Value: i32): i32
{
  puts("third");
  return Value;
}

// The function body must execute only after all three arguments are evaluated.
func combine(First: i32, Second: i32, Third: i32): i32
{
  puts("combine");
  return First + First + First + First + Second + Second + Third;
}

// An explicit void return resumes execution in the caller.
func explicitVoid(): void
{
  puts("explicit");
  return;
}

// Falling off a void body executes the synthesized return.
func implicitVoid(): void
{
  puts("implicit");
}

// An early void return skips the remaining output while the other path falls through.
func conditionalVoid(Stop: bool): void
{
  if (Stop)
  {
    puts("early");
    return;
  }
  puts("fallthrough");
}

// Storing and repeatedly reading a call result must not replay its body.
func reusedResult(): i32
{
  var Value = first(7);
  return Value + Value + Value;
}

// Every result checkpoint is independent from the expected call trace.
func check(Condition: bool): bool
{
  if (Condition)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  // Evaluate arguments left to right before entering the callee body.
  puts("FunctionEffects.argument_order");
  if (!check(combine(first(1), second(2), third(3)) == 11)) return 1;

  // Complete each nested argument before advancing to the next argument.
  puts("FunctionEffects.nested_argument_order");
  if (!check(combine(first(second(4)), second(third(5)), third(first(6))) == 32)) return 1;

  // A binary expression evaluates both calls once and in source order.
  puts("FunctionEffects.expression_order");
  if (!check(first(7) + second(8) == 15)) return 1;

  // Multiple reads of a stored return value produce a single operand trace.
  puts("FunctionEffects.reuse_result");
  if (!check(reusedResult() == 21)) return 1;

  // Ignoring a non-void result still executes the call exactly once.
  puts("FunctionEffects.discard_result");
  first(9);
  if (!check(true)) return 1;

  // Both explicit and synthesized void returns allow the caller to continue.
  puts("FunctionEffects.void_returns");
  explicitVoid();
  implicitVoid();
  if (!check(true)) return 1;

  // Only the chosen early-return path produces output for each invocation.
  puts("FunctionEffects.conditional_void");
  conditionalVoid(true);
  conditionalVoid(false);
  if (!check(true)) return 1;

  return 0;
}

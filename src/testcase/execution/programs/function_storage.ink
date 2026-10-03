// Check local lifetime, frame isolation and control flow through ordinary calls.
import "C" func puts(Text: *u8): i32;

// Updating a local copy leaves the caller's argument and prior results unchanged.
func bump(Input: i32): i32
{
  var Local = Input;
  Local = Local + 3;
  return Local;
}

// Locals in separate activations do not overwrite each other's stored values.
func nestedLocals(Input: i32): i32
{
  var First = bump(Input);
  var Second = bump(First);
  return First + First + Second;
}

// A returned value remains independent after its source local is changed.
func copiedReturn(): i32
{
  var Input: i32 = 4;
  var Saved = bump(Input);
  Input = 20;
  var Later = bump(Input);
  return Saved + Saved + Later;
}

// Early returns cover negative and zero inputs while positives reach the final return.
func classify(Value: i32): i32
{
  if (Value < 0) return 11;
  if (Value == 0) return 22;
  return 33;
}

// Both branches return directly without requiring a trailing fallback return.
func branchReturn(Condition: bool): i32
{
  if (Condition)
  {
    return 17;
  }
  else
  {
    return 29;
  }
}

// Recursive callers retain their own mutable local after their child returns.
func recursiveLocals(Depth: i32): i32
{
  var Local = Depth;
  if (Depth == 0) return Local;
  var Child = recursiveLocals(Depth + -1);
  return Local + Child;
}

// A successful assertion prints exactly one PASS and failure propagates to main.
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
  // A callee's mutated local cannot update the caller's by-value input.
  puts("Storage.by_value_argument");
  var Input: i32 = 5;
  if (!check(bump(Input) == 8 && Input == 5)) return 1;

  // Repeated invocations start with independent local storage.
  puts("Storage.repeated_calls");
  if (!check(bump(5) == 8 && bump(20) == 23 && bump(5) == 8)) return 1;

  // Nested callees leave both outer results intact.
  puts("Storage.nested_frames");
  if (!check(nestedLocals(5) == 27)) return 1;

  // A fresh outer activation does not retain the previous activation's locals.
  puts("Storage.fresh_outer_frame");
  if (!check(nestedLocals(20) == 72)) return 1;

  // Saved return values survive later local assignment and another call.
  puts("Storage.return_value_copy");
  if (!check(copiedReturn() == 37)) return 1;

  // The first conditional return handles negative inputs.
  puts("Storage.early_negative_return");
  if (!check(classify(-3) == 11)) return 1;

  // The second conditional return handles zero after the first condition fails.
  puts("Storage.early_zero_return");
  if (!check(classify(0) == 22)) return 1;

  // A positive input reaches the final return after both guards fail.
  puts("Storage.final_return");
  if (!check(classify(8) == 33)) return 1;

  // Both sides of a fully returning if/else execute through ordinary calls.
  puts("Storage.both_branches_return");
  if (!check(branchReturn(true) == 17 && branchReturn(false) == 29)) return 1;

  // Recursive storage is separate even when every activation uses the same names.
  puts("Storage.recursive_locals");
  if (!check(recursiveLocals(5) == 15)) return 1;

  return 0;
}

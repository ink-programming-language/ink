// Run typed calls from main and verify each returned value before reporting PASS.
import "C" func puts(Text: *u8): i32;

// A zero-argument call returns a scalar value to its caller.
func constant(): i32
{
  return 37;
}

// Positional arguments remain distinct when the callee uses each one.
func combine(First: i32, Second: i32, Third: i32): i32
{
  return First + First + First + First + Second + Second + Third;
}

// Calls may receive and return values from other ordinary calls.
func add(Left: i32, Right: i32): i32
{
  return Left + Right;
}

// A typed i32 argument chooses the i32 member of an overload set.
func pick(Value: i32): i32
{
  return Value + 100;
}

// A typed u8 argument chooses the u8 member without widening it to i32.
func pick(Value: u8): i32
{
  if (Value == 7) return 207;
  return 200;
}

// Boolean arguments participate in exact-type overload selection.
func pick(Value: bool): i32
{
  if (Value) return 301;
  return 300;
}

// Arity distinguishes this zero-argument overload from its typed siblings.
func pick(): i32
{
  return 400;
}

// Boolean parameters and returns preserve both truth values through local storage.
func echoBool(Value: bool): bool
{
  var Copy = Value;
  return Copy;
}

// A nested declaration executes with its own explicit parameters.
func nestedDeclaration(Value: i32): i32
{
  func inner(Input: i32): i32
  {
    return Input + 4;
  }
  return inner(Value);
}

// A wide value crosses ordinary call boundaries without truncation.
func echoWide(Value: u128): u128
{
  var Copy = Value;
  return Copy;
}

// Recursive activations terminate at a base case and preserve pending additions.
func recursiveSum(Value: i32): i32
{
  if (Value <= 0) return 0;
  return Value + recursiveSum(Value + -1);
}

// Each call site has an exact stdout checkpoint as well as an exit-status assertion.
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
  // Exercise a call with an empty argument list.
  puts("Function.zero_arguments");
  if (!check(constant() == 37)) return 1;

  // Preserve the positional ordering of three scalar arguments.
  puts("Function.multiple_arguments");
  if (!check(combine(2, 4, 6) == 22)) return 1;

  // Feed call results into both arguments of an outer call.
  puts("Function.nested_calls");
  if (!check(add(add(1, 2), add(3, 4)) == 10)) return 1;

  // Parentheses around a direct callee retain its callable identity.
  puts("Function.parenthesized_callee");
  if (!check((add)(5, 8) == 13)) return 1;

  // Resolve a typed signed local to its matching overload.
  puts("Function.overload_i32");
  var Signed: i32 = 7;
  if (!check(pick(Signed) == 107)) return 1;

  // Prefer the ordinary i32 overload for an untyped integer literal.
  puts("Function.overload_literal");
  if (!check(pick(7) == 107)) return 1;

  // Resolve a typed narrow unsigned local to its matching overload.
  puts("Function.overload_u8");
  var Unsigned: u8 = 7;
  if (!check(pick(Unsigned) == 207)) return 1;

  // Distinguish the boolean overload from both integer overloads.
  puts("Function.overload_bool");
  if (!check(pick(true) == 301 && pick(false) == 300)) return 1;

  // Resolve the same name using argument count before runtime execution.
  puts("Function.overload_arity");
  if (!check(pick() == 400)) return 1;

  // Return true and false through an ordinary function and a mutable local.
  puts("Function.boolean_payload");
  if (!check(echoBool(true) && !echoBool(false))) return 1;

  // Execute a function owned by the caller's declaration scope.
  puts("Function.nested_declaration");
  if (!check(nestedDeclaration(9) == 13)) return 1;

  // Exercise all 128 bits in an unsigned parameter and returned result.
  puts("Function.wide_payload");
  var Wide: u128 = 340282366920938463463374607431768211455;
  if (!check(echoWide(Wide) == Wide)) return 1;

  // Take the recursive base case without creating an additional activation.
  puts("Function.recursive_base");
  if (!check(recursiveSum(0) == 0)) return 1;

  // Retain each frame's argument until its recursive child returns.
  puts("Function.recursive_frames");
  if (!check(recursiveSum(6) == 21)) return 1;

  return 0;
}

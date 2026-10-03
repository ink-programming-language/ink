// Runtime addition composes with locals, branches, calls and left-to-right operand effects.
import "C" func puts(Text: *u8): i32;

// Parentheses and left association preserve the same result for nested addition.
func grouping(First: i32, Second: i32, Third: i32): bool
{
  return First + Second + Third == 42 && (First + Second) + Third == 42 && First + (Second + Third) == 42;
}

// Unary signs on literals bind before addition, including parenthesized negative literals.
func signedLiterals(): i32
{
  return +20 + -7 + -(3) + +2;
}

// Untyped locals acquire the default integer type and remain usable in subsequent additions.
func inferredLocals(): i32
{
  var First = 11;
  var Second = First + 13;
  var Third = Second + First;
  return Third + 7;
}

// Assignment reloads the previous local value before replacing its storage.
func repeatedAssignment(): i32
{
  var Value: i32 = 3;
  Value = Value + 5;
  Value = Value + Value;
  Value = Value + -6;
  return Value;
}

// A shadowed local retains its own storage while the surrounding variable is updated.
func shadowedLocals(): i32
{
  var Value: i32 = 10;
  var Result: i32 = 0;
  {
    var Value: i32 = 20;
    Result = Value + 1;
  }
  Value = Value + Result;
  return Value;
}

// Both branch paths store an arithmetic result that is reloaded after the merge.
func branchAssignment(Positive: bool): i32
{
  var Value: i32 = 7;
  if (Positive)
  {
    Value = Value + 5;
  }
  else
  {
    Value = Value + -5;
  }
  return Value + 1;
}

func firstOperand(): i32
{
  puts("first");
  return 5;
}

func secondOperand(): i32
{
  puts("second");
  return 7;
}

func thirdOperand(): i32
{
  puts("third");
  return 11;
}

// Each nested addition operand executes once in source order; stdout checks that order.
func operandOrder(): i32
{
  return firstOperand() + (secondOperand() + thirdOperand());
}

func check(Actual: bool): bool
{
  if (Actual)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func main(): i32
{
  puts("Arithmetic.grouping");
  if (!check(grouping(10, 13, 19))) return 1;

  puts("Arithmetic.signed_literals");
  if (!check(signedLiterals() == 12)) return 1;

  puts("Arithmetic.inferred_locals");
  if (!check(inferredLocals() == 42)) return 1;

  puts("Arithmetic.repeated_assignment");
  if (!check(repeatedAssignment() == 10)) return 1;

  puts("Arithmetic.shadowed_locals");
  if (!check(shadowedLocals() == 31)) return 1;

  puts("Arithmetic.branch_positive");
  if (!check(branchAssignment(true) == 13)) return 1;

  puts("Arithmetic.branch_negative");
  if (!check(branchAssignment(false) == 3)) return 1;

  puts("Arithmetic.operand_order");
  if (!check(operandOrder() == 23)) return 1;

  return 0;
}

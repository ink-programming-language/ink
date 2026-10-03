// Exercise every three-input combination with independent expected results for nested logical expressions.
import "C" func puts(Text: *u8): i32;

func check(Actual: bool, Expected: bool): bool
{
  if (Actual == Expected)
  {
    puts("PASS");
    return true;
  }
  puts("FAIL");
  return false;
}

func checkCombination(A: bool, B: bool, C: bool, Precedence: bool, Negation: bool, Merge: bool): bool
{
  if (!check(!A || B && C, Precedence)) return false;
  if (!check(!(A || B) && !C, Negation)) return false;
  if (!check((A && B) || (!B && C), Merge)) return false;
  return true;
}

func main(): i32
{
  // The three expected columns check precedence, grouped negation and two converging short-circuit paths.
  puts("Nested.false.false.false");
  if (!checkCombination(false, false, false, true, true, false)) return 1;

  puts("Nested.false.false.true");
  if (!checkCombination(false, false, true, true, false, true)) return 1;

  puts("Nested.false.true.false");
  if (!checkCombination(false, true, false, true, false, false)) return 1;

  puts("Nested.false.true.true");
  if (!checkCombination(false, true, true, true, false, false)) return 1;

  puts("Nested.true.false.false");
  if (!checkCombination(true, false, false, false, false, false)) return 1;

  puts("Nested.true.false.true");
  if (!checkCombination(true, false, true, false, false, true)) return 1;

  puts("Nested.true.true.false");
  if (!checkCombination(true, true, false, false, false, true)) return 1;

  puts("Nested.true.true.true");
  if (!checkCombination(true, true, true, true, false, true)) return 1;

  return 0;
}

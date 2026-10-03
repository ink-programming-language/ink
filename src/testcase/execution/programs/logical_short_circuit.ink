// Run every case from main and assert its result before reporting PASS on stdout.
import "C" func puts(Text: *u8): i32;

// Reaching this body would fail native lookup, exposing any eager evaluation of a skipped right operand.
import "C" func InkMissingLogicalShortCircuitSymbol71e4935b(): bool;

func missing(): bool
{
  return InkMissingLogicalShortCircuitSymbol71e4935b();
}

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func andMissing(Left: bool): bool
{
  return Left && missing();
}

func orMissing(Left: bool): bool
{
  return Left || missing();
}

func skip_and(): i32
{
  return encode(andMissing(false));
}

func skip_or(): i32
{
  return encode(orMissing(true));
}

func nested_skip(): i32
{
  return encode((false && missing()) || (true || missing()));
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
  puts("ShortCircuit.skip_and");
  if (!check(skip_and(), 23)) return 1;

  puts("ShortCircuit.skip_or");
  if (!check(skip_or(), 17)) return 1;

  puts("ShortCircuit.nested_skip");
  if (!check(nested_skip(), 17)) return 1;

  return 0;
}

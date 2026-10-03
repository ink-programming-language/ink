// A false left operand requires the right call, whose body resolves a deliberately missing native symbol.
import "C" func InkMissingLogicalOrSymbol71e4935b(): bool;

func missing(): bool
{
  return InkMissingLogicalOrSymbol71e4935b();
}

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func orMissing(Left: bool): bool
{
  return Left || missing();
}

func main(): i32
{
  return encode(orMissing(false));
}

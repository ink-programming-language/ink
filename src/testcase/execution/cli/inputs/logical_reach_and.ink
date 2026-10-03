// A true left operand requires the right call, whose body resolves a deliberately missing native symbol.
import "C" func InkMissingLogicalAndSymbol71e4935b(): bool;

func missing(): bool
{
  return InkMissingLogicalAndSymbol71e4935b();
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

func main(): i32
{
  return encode(andMissing(true));
}

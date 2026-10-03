// Nested short-circuit continuations eventually execute the wrapper around a missing native symbol.
import "C" func InkMissingLogicalNestedSymbol71e4935b(): bool;

func missing(): bool
{
  return InkMissingLogicalNestedSymbol71e4935b();
}

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func main(): i32
{
  return encode((false || true) && (false || missing()));
}

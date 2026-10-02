// A true left operand requires the right call, which deliberately has no body.
func missing(): bool;

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

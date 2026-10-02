// Nested short-circuit continuations eventually reach the deliberately missing function.
func missing(): bool;

func encode(Value: bool): i32
{
  if (Value) return 17;
  return 23;
}

func main(): i32
{
  return encode((false || true) && (false || missing()));
}

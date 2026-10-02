// This definition must be rejected even though main never calls it.
func invalid(Value: i32): bool
{
  return Value || false;
}

func main(): i32
{
  return 0;
}

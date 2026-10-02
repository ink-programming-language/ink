// This definition must be rejected even though main never calls it.
func invalid(Left: bool, Right: bool): bool
{
  return Left <= Right;
}

func main(): i32
{
  return 0;
}

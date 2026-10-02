// This definition must be rejected even though main never calls it.
func invalid(Left: i8, Right: i16): bool
{
  return Left == Right;
}

func main(): i32
{
  return 0;
}

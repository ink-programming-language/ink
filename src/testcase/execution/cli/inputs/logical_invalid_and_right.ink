// This definition must be rejected even though main never calls it.
func invalid(): bool
{
  return false && 1;
}

func main(): i32
{
  return 0;
}

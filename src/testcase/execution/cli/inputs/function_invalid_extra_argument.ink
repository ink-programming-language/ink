// A zero-parameter function rejects an unexpected argument.
func constant(): i32
{
  return 7;
}

func main(): i32
{
  return constant(1);
}

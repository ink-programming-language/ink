// An explicit return without a value is invalid in a non-void function.
func empty(): i32
{
  return;
}

func main(): i32
{
  return 0;
}

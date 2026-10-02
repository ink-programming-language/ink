// Falling off a non-void function must report a missing return value.
func missing(): i32
{
  var Value: i32 = 7;
}

func main(): i32
{
  return 0;
}

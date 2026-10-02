// A call with no arguments cannot satisfy one required positional parameter.
func identity(Value: i32): i32
{
  return Value;
}

func main(): i32
{
  return identity();
}

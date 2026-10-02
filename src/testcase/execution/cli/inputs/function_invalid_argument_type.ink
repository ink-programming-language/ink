// A boolean argument is not implicitly converted to an integer parameter.
func identity(Value: i32): i32
{
  return Value;
}

func main(): i32
{
  return identity(true);
}

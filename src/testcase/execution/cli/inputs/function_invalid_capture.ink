// A nested function must not implicitly capture its parent's runtime parameter.
func outer(Value: i32): i32
{
  func inner(): i32
  {
    return Value;
  }
  return 0;
}

func main(): i32
{
  return 0;
}

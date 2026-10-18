func negate(Value: i32): i32
{
  return -Value;
}

func invoke(Callback: func(i32): i32, Value: i32): i32
{
  return Callback(Value);
}

func main(): i32
{
  return invoke(negate, 42);
}
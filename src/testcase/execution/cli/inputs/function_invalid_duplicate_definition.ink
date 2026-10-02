// Two definitions with identical names and parameter types cannot form an overload set.
func duplicate(Value: i32): i32
{
  return Value;
}

func duplicate(Other: i32): i32
{
  return Other;
}

func main(): i32
{
  return duplicate(7);
}

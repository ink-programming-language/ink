// A boolean argument matches neither candidate in an integer-only overload set.
func pick(Value: i32): i32
{
  return 1;
}

func pick(Value: u8): i32
{
  return 2;
}

func main(): i32
{
  return pick(true);
}

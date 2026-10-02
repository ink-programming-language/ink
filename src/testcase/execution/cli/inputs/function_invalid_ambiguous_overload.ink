// A fitting literal cannot choose between equally ranked signed and unsigned narrow overloads.
func pick(Value: i8): i32
{
  return 1;
}

func pick(Value: u8): i32
{
  return 2;
}

func main(): i32
{
  return pick(7);
}

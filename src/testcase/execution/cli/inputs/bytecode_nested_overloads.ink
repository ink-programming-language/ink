// Identical nested names under distinct parent overloads must retain different bytecode identities.
func outer(Value: i32): i32
{
  func inner(): i32
  {
    return 19;
  }
  return inner();
}

func outer(Value: u8): i32
{
  func inner(): i32
  {
    return 23;
  }
  return inner();
}

func main(): i32
{
  var Small: u8 = 0;
  return outer(0) + outer(Small);
}

// Initialization on an arm that already returned does not initialize the surviving path.
func choose(Flag: bool): i32
{
  var Result: i32;
  if (Flag)
  {
    Result = 7;
    return Result;
  }
  return Result;
}

func main(): i32
{
  return choose(false);
}

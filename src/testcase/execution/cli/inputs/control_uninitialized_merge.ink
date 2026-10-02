// An assignment on only one branch cannot initialize the common continuation.
func choose(Flag: bool): i32
{
  var Result: i32;
  if (Flag) Result = 7;
  return Result;
}

func main(): i32
{
  return choose(false);
}

// A typed i8 argument is not implicitly widened to an i32 parameter.
func identity(Value: i32): i32
{
  return Value;
}

func main(): i32
{
  var Narrow: i8 = 7;
  return identity(Narrow);
}

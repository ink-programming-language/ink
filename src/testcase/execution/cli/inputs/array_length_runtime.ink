// A runtime parameter cannot determine a fixed array length.
func build(Count: i32): i32
{
  var Values = [0; Count];
  return 0;
}

func main(): i32
{
  return build(2);
}

// A valid ordinary declaration still fails when execution reaches its absent body.
func declared(Value: i32): i32;

func main(): i32
{
  return declared(7);
}

// Ordinary Ink declarations require a body and fail during source analysis before main executes.
func declared(Value: i32): i32;

func main(): i32
{
  return declared(7);
}

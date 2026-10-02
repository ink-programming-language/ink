// A native declaration retains its required parameter count during overload resolution.
extern "C" func abs(Value: i32): i32;

func main(): i32
{
  return abs();
}

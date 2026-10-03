// A boolean cannot be passed to the declared i32 C parameter.
import "C" func abs(Value: i32): i32;

func main(): i32
{
  return abs(true);
}

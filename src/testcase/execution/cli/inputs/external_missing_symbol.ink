// Calling a C declaration whose process symbol does not exist reports INK-E0014.
import "C" func InkMissingExternalProgramSymbola57e3f8d(Value: i32): i32;

func main(): i32
{
  return InkMissingExternalProgramSymbola57e3f8d(1);
}

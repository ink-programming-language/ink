// Compile-time native symbol lookup reports the same explicit missing-symbol error.
import "C" func InkMissingExternalProgramSymbola57e3f8d(Value: i32): i32;

func main(): i32
{
  return comptime InkMissingExternalProgramSymbola57e3f8d(1);
}

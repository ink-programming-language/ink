// The FFI rejects a 128-bit return signature before invoking the resolved C symbol.
import "C" func abs(Value: i32): i128;

func main(): i32
{
  abs(-1);
  return 0;
}

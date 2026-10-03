// The FFI rejects f16 before the deliberately incompatible abs declaration can run.
import "C" func abs(Value: i32): f16;

func main(): i32
{
  abs(-1);
  return 0;
}

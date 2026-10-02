// A divisor that evaluates to zero reports the arithmetic diagnostic during compile-time evaluation.
func main(): i32
{
  return comptime (42 / (3 - 3));
}

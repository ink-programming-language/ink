// A left-shift count equal to the i32 bit width is outside the valid range.
func main(): i32
{
  return comptime (1 << 32);
}

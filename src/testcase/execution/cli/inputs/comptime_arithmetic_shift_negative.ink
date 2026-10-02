// A negative right-shift count is rejected before any integer result is produced.
func main(): i32
{
  return comptime (8 >> -1);
}

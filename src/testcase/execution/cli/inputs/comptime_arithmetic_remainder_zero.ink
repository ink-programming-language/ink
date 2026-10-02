// Remainder rejects a zero divisor just like division.
func main(): i32
{
  return comptime (42 % 0);
}

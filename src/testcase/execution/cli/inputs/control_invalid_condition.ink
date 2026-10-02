// Integer conditions must be rejected instead of being converted to bool.
func main(): i32
{
  if (1) return 0;
  return 1;
}

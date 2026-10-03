// Compile-time indexed assignment rejects an index equal to the array length.
func main(): i32
{
  comptime var Values = [42];
  comptime
  {
    Values[1] = 0;
  }
  return 0;
}

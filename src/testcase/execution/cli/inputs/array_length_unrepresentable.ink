// A length exceeding the unsigned 64-bit range is rejected before materialization.
func main(): i32
{
  var Values = [0; 18446744073709551616];
  return 0;
}

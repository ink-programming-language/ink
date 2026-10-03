// A mutable u128 index above 64 bits must not truncate to the valid low-word index zero.
func main(): i32
{
  var Values = [10, 20];
  var Index: u128 = 18446744073709551616;
  return Values[Index];
}

// Even index zero is out of bounds for an explicitly typed empty array at runtime.
func main(): i32
{
  var Values: [i32; 0] = [];
  var Index: i32 = 0;
  return Values[Index];
}

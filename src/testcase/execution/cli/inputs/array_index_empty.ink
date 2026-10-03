// A zero-length array has no valid element even at constant index zero.
func main(): i32
{
  var Values: [i32; 0] = [];
  return Values[0];
}

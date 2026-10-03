// A constant out-of-bounds indexed write is rejected before generating a store.
func main(): i32
{
  var Values = [42];
  Values[1] = 0;
  return 0;
}

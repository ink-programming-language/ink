// A mutable negative index must reach the runtime bounds check before any element is read.
func main(): i32
{
  var Values = [10, 20];
  var Index: i32 = -1;
  return Values[Index];
}

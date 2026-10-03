// Reading the one-past element through a mutable index must report a runtime bounds error.
func main(): i32
{
  var Values = [10, 20];
  var Index: i32 = 2;
  return Values[Index];
}

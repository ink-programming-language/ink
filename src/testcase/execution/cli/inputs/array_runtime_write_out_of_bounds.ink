// Writing the one-past element must fail without treating adjacent memory as an array element.
func main(): i32
{
  var Values = [10, 20];
  var Index: i32 = 2;
  Values[Index] = 30;
  return 0;
}

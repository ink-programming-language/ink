// Forming an indexed address must reject a mutable one-past index before pointer dereference.
func main(): i32
{
  var Values = [10, 20];
  var Index: i32 = 2;
  var Address = &Values[Index];
  return *Address;
}

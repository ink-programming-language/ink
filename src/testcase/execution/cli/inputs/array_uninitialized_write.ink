// A partial element assignment does not initialize an otherwise uninitialized whole array.
func main(): i32
{
  var Values: [i32; 2];
  Values[0] = 42;
  return 0;
}

// A repeated array rejects a negative compile-time length.
func main(): i32
{
  var Values = [0; -1];
  return 0;
}

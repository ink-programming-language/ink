// A negative constant subscript fails during semantic analysis.
func main(): i32
{
  var Values = [42];
  return Values[-1];
}

// A floating-point array subscript is rejected without an implicit integer conversion.
func main(): i32
{
  var Values = [42];
  return Values[0.0];
}

// Taking an address still requires a valid array element rather than a one-past index.
func main(): i32
{
  var Values = [42];
  var Address = &Values[1];
  return 0;
}

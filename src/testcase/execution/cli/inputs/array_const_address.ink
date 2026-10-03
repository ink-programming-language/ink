// Taking a writable element address cannot bypass a const array binding.
func main(): i32
{
  const Values = [1];
  var Address = &Values[0];
  return 0;
}

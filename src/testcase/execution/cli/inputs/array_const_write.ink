// An element of a const array cannot be assigned a new value.
func main(): i32
{
  const Values = [1];
  Values[0] = 2;
  return 0;
}

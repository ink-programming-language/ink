// A representable 64-bit length still fails the materialization limit before allocating its elements.
func main(): i32
{
  var Values = [0; 18446744073709551615];
  return 0;
}

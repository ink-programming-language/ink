// Contextually typed byte-array elements reject integer literals above 255 instead of truncating.
func main(): i32
{
  var Values: [u8; 1] = [256];
  return 0;
}

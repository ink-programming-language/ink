// Unknown member names cannot be translated into arbitrary field offsets.
class Point
{
  field X: i32 = 42;
};
func main(): i32
{
  return Point().Missing;
}

// Reading a private field outside its declaring class is rejected.
class Point
{
  private field X: i32 = 42;
};
func main(): i32
{
  return Point().X;
}

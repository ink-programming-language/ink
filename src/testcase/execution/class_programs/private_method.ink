// Calling a private method outside its declaring class is rejected.
class Point
{
  private func hidden(): i32
  {
    return 42;
  }
};
func main(): i32
{
  return Point().hidden();
}

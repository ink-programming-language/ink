// The initial method subset is mutable and cannot receive a readonly object.
class Point
{
  field X: i32 = 42;
  func read(): i32
  {
    return this.X;
  }
};
func main(): i32
{
  var Value = Point();
  return Value.read();
}

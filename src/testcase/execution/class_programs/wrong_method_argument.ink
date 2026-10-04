// Method overload selection rejects a boolean argument for an integer parameter.
class Point
{
  field X: i32 = 40;
  func add(N: i32): i32
  {
    return this.X + N;
  }
};
func main(): i32
{
  var Value = Point();
  return Value.add(true);
}

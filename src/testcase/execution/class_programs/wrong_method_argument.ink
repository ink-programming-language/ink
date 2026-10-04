// Method overload selection rejects a boolean argument for an integer parameter.
class Point
{
  field X: i32 = 40;
  func add(N: i32): i32
  {
    return this.X + N;
  }

  func __init__(): void
  {
  }

  func __init__(InitialX: i32): void
  {
    this.X = InitialX;
  }
};
func main(): i32
{
  var Value = Point();
  return Value.add(true);
}

// A module-qualified constructor and signature refer to the same nominal class as a from import.
class Point
{
  field X: i32 = 40;

  func add(N: i32): i32
  {
    this.X = this.X + N;
    return this.X;
  }
};

func echo(Value: Point): Point
{
  return Value;
}

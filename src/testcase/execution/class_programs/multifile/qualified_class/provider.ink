// A module-qualified constructor and signature refer to the same nominal class as a from import.
class Point
{
  field X: i32 = 40;

  func add(N: i32): i32
  {
    this.X = this.X + N;
    return this.X;
  }

  func __init__(): void
  {
  }

  func __init__(InitialX: i32): void
  {
    this.X = InitialX;
  }
};

func echo(Value: Point): Point
{
  return Value;
}

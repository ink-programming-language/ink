// Imported construction calls a default helper and public methods reach private implementation functions.
private func seed(): i32
{
  return 38;
}

class Point
{
  field X: i32 = seed();
  private field Y: i32 = 2;

  func add(N: i32): i32
  {
    this.X = this.X + N;
    return this.total();
  }

  private func total(): i32
  {
    return this.X + this.Y;
  }

  func __init__(): void
  {
  }

  func __init__(InitialX: i32): void
  {
    this.X = InitialX;
  }

  private func __init__(InitialX: i32, InitialY: i32): void
  {
    this.X = InitialX;
    this.Y = InitialY;
  }
};

comptime func frozen(): Point
{
  var Result = Point();
  Result.add(2);
  return Result;
}

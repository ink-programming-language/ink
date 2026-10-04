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
};

comptime func frozen(): Point
{
  var Result = Point();
  Result.add(2);
  return Result;
}

// Exporting the class does not publish its private instance methods.
class Point
{
  field X: i32 = 42;

  private func hidden(): i32
  {
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

// Exporting the class does not publish its private instance methods.
class Point
{
  field X: i32 = 42;

  private func hidden(): i32
  {
    return this.X;
  }
};

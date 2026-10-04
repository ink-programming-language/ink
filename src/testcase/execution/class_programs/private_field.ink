// Reading a private field outside its declaring class is rejected.
class Point
{
  private field X: i32 = 42;

  func __init__(): void
  {
  }

  private func __init__(InitialX: i32): void
  {
    this.X = InitialX;
  }
};
func main(): i32
{
  return Point().X;
}

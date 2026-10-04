// Unknown member names cannot be translated into arbitrary field offsets.
class Point
{
  field X: i32 = 42;

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
  return Point().Missing;
}

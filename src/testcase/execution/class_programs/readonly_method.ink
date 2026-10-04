// Mutable methods cannot be called through a const object binding.
class Point
{
  field X: i32 = 42;

  func read(): i32
  {
    return this.X;
  }
};

func main(): i32
{
  const Value = Point();
  return Value.read();
}

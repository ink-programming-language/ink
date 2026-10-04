// Defaults bind in the defining scope, execute once, and methods mutate the original receiver.
import "C" func puts(Text: *u8): i32;

func defaultY(): i32
{
  puts("default");
  return 2;
}

class Point
{
  field X: i32 = 40;
  field Y: i32 = defaultY();

  func add(N: i32): i32
  {
    this.X = this.X + N;
    return this.read();
  }

  private func hidden(): i32
  {
    return this->X + this.Y;
  }

  func read(): i32
  {
    return this.hidden();
  }
};

func main(): i32
{
  var defaultY = 99;
  puts("Class.defaults");
  var A = Point(38);
  if (A.add(2) != 42 || A.X != 40)
  {
    return 1;
  }
  puts("PASS");
  puts("Class.pointer_method");
  var B = Point();
  var Address = &B;
  if (Address->read() != 42)
  {
    return 2;
  }
  puts("PASS");
  puts("Class.temporary_method");
  if (Point(38, 2).add(2) != 42)
  {
    return 3;
  }
  puts("PASS");
  return 0;
}

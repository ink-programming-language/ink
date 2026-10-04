// Compile-time class methods and nested mutations freeze into class constants consumed by runtime code.
import "C" func puts(Text: *u8): i32;

class Point
{
  field X: i32 = 20;

  func add(N: i32): void
  {
    this.X = this.X + N;
  }

  func __init__(): void
  {
  }

  func __init__(InitialX: i32): void
  {
    this.X = InitialX;
  }
};

class Box
{
  field Items: [Point; 2];

  func __init__(InitialItems: [Point; 2]): void
  {
    this.Items = InitialItems;
  }
};

comptime func make(): Point
{
  var Result = Point(40);
  Result.add(2);
  return Result;
}

comptime var Global = Box([Point(1), Point(40)]);
comptime Global.Items[1].add(2);

func main(): i32
{
  puts("Class.comptime_return");
  const Frozen = comptime make();
  var Runtime = Frozen;
  Runtime.X = 7;
  if (Frozen.X != 42 || Runtime.X != 7)
  {
    return 1;
  }
  puts("PASS");
  puts("Class.comptime_nested");
  if (Global.Items[1].X != 42 || Global.Items[0].X != 1)
  {
    return 2;
  }
  puts("PASS");
  return 0;
}

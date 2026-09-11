// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(x, y);
  if (((x < 10) && (y < 10)))
  {
    write((x * y));
  } else
  {
    write("-1");
  }
}

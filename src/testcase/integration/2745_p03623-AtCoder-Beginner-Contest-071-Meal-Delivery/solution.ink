// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(x, a, b);
  if ((abs((x - a)) < abs((x - b))))
  {
    write(cpp_char("A"), "\n");
  } else
  {
    write(cpp_char("B"), "\n");
  }
  return 0;
}

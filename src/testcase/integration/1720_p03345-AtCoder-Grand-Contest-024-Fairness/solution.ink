// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(a, b, c, k);
  if (((k % 2) == 0))
  {
    write((a - b), "\n");
  } else
  {
    write((b - a), "\n");
  }
}

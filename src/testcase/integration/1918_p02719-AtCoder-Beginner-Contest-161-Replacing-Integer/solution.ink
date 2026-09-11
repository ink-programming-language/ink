// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var k1: dynamic = cpp_uninitialized();
  read(a, b);
  k1 = (a % b);
  write(min(k1, (b - k1)), "\n");
}

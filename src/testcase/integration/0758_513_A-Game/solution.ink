// Translated from solution.cpp.

func main() -> dynamic
{
  var n1: dynamic = cpp_uninitialized();
  var n2: dynamic = cpp_uninitialized();
  var k1: dynamic = cpp_uninitialized();
  var k2: dynamic = cpp_uninitialized();
  read(n1, n2, k1, k2);
  if ((n1 <= n2))
  {
    write("Second");
  } else
  {
    write("First");
  }
  return 0;
}

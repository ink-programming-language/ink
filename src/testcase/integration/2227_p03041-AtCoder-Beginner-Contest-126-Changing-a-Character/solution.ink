// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var s: dynamic = cpp_array(51);
  read(s);
  s[(k - 1)] = ((s[(k - 1)] - cpp_char("A")) + cpp_char("a"));
  write(s, "\n");
}

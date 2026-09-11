// Translated from solution.cpp.

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(k, a, b);
  var g1: dynamic = (a / k);
  var g2: dynamic = (b / k);
  var mod2: dynamic = (b % k);
  var mod1: dynamic = (a % k);
  if (((mod1 != 0) && (g2 == 0)))
  {
    write("-1");
    return 0;
  }
  if (((mod2 != 0) && (g1 == 0)))
  {
    write("-1");
    return 0;
  }
  write((g1 + g2));
  return 0;
}

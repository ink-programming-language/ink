// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var num: dynamic = 0;
  while ((b > 0))
  {
    b = (b - a);
    num += 1;
  }
  write(num, "\n");
  return 0;
}

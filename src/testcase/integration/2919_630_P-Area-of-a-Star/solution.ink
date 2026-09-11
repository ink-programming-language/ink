// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  read(n, r);
  var a: dynamic = ((atan(1) * 2) / n);
  ans = (((((r * r) * n) * sin((2 * a))) * sin(a)) / sin(((((n * 2) - 3)) * a)));
  printf("%.10f\n", ans);
  return 0;
}

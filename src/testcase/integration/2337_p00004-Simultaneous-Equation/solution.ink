// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  while (((((((cin >> a) >> b) >> c) >> d) >> e) >> f))
  {
    y = ((((c * d) - (a * f))) / (((b * d) - (a * e))));
    x = (((c - (b * y))) / a);
    printf("%.3f %.3f\n", x, y);
  }
  return 0;
}

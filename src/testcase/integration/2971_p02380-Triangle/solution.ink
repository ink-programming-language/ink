// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var cr: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_uninitialized();
  var H: dynamic = cpp_uninitialized();
  read(a, b, c);
  cr = (((acos(-1.0) * c)) / 180.0);
  S = (((0.5 * a) * b) * sin(cr));
  L = ((a + b) + sqrt((((a * a) + (b * b)) - (((2 * a) * b) * cos(cr)))));
  H = (b * sin(cr));
  printf("%lf\n%lf\n%lf\n", S, L, H);
  return 0;
}

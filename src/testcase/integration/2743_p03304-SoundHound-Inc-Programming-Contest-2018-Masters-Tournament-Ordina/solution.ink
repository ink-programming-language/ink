// Translated from solution.cpp.

var ld: dynamic = dynamic;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m, d);
  var a: dynamic = 2.0;
  if ((!d))
  {
    a = 1.0;
  }
  printf("%.7Lf", ((((a * ((n - d))) * ((m - 1)))) / ((n * n))));
  return 0;
}

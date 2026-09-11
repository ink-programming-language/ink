// Translated from solution.cpp.

var INF: dynamic = 999999999999999999;

var PI: dynamic = acos(-1.0);

func stop() -> dynamic
{
  exit(0);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(n, r);
  var sn: dynamic = sin((PI / n));
  var x: dynamic = ((((2.0 * r) * sn)) / ((2.0 - (2.0 * sn))));
  printf("%.9lf", x);
  stop();
}

// Translated from solution.cpp.

var USE_MATH_DEFINES: dynamic = cpp_expression("#def");

var INF: dynamic = 1e18;

var MOD: dynamic = (1e9 + 7);

func extGCD(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  var d: dynamic = a;
  if ((b == 0))
  {
    x = 1;
    y = 0;
  } else
  {
    d = extGCD(b, (a % b), y, x);
    y -= ((a / b) * x);
  }
  return d;
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var G: dynamic = cpp_uninitialized();
  read(a, b);
  G = extGCD(a, b, x, y);
  write(x, " ", y, "\n");
  return 0;
}

// Translated from solution.cpp.

var USE_MATH_DEFINES = cpp_expression("#def");

var INF = 1e18;

var MOD = (1e9 + 7);

func extGCD(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  var d = a;
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

func main()
{
  var a: dynamic;
  var b: dynamic;
  var x: dynamic;
  var y: dynamic;
  var G: dynamic;
  read(a, b);
  G = extGCD(a, b, x, y);
  write(x, " ", y, "\n");
  return 0;
}

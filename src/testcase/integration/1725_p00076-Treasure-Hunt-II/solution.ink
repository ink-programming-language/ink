// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(long long i=(a);i<(b);i++)");
}

func REP(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(long long i=0;i<(N);i++)");
}

func ALL(s: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #");
}

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var PI: dynamic = cpp_expression("#include <");

var INF: dynamic = cpp_expression("#include <");

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while (((cin >> n) && (n != -1)))
  {
    var x: dynamic = 1;
    var y: dynamic = 0;
    var angle: dynamic = 0;
    REP(i, (n - 1));
    {
      angle += atan((1.0 / sqrt((pow(x, 2) + pow(y, 2)))));
      var px: dynamic = x;
      var py: dynamic = y;
      x = ((sqrt(((1 + pow(px, 2)) + pow(py, 2)))) * cos(angle));
      y = ((sqrt(((1 + pow(px, 2)) + pow(py, 2)))) * sin(angle));
    }
    write(x, "\n");
    write(y, "\n");
  }
}

// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(long long i=(a);i<(b);i++)");
}

func REP(i: dynamic, N: dynamic)
{
  cpp_macro("for(long long i=0;i<(N);i++)");
}

func ALL(s: dynamic)
{
  return cpp_expression("#include <iostream> #");
}

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var PI = cpp_expression("#include <");

var INF = cpp_expression("#include <");

var n: dynamic;

func main()
{
  while (((cin >> n) && (n != -1)))
  {
    var x = 1;
    var y = 0;
    var angle = 0;
    REP(i, (n - 1));
    {
      angle += atan((1.0 / sqrt((pow(x, 2) + pow(y, 2)))));
      var px = x;
      var py = y;
      x = ((sqrt(((1 + pow(px, 2)) + pow(py, 2)))) * cos(angle));
      y = ((sqrt(((1 + pow(px, 2)) + pow(py, 2)))) * sin(angle));
    }
    write(x, "\n");
    write(y, "\n");
  }
}

// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var pb: dynamic = cpp_expression("#include <bi");

var a: dynamic = cpp_array((1 << 10));

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var ai: dynamic = cpp_uninitialized();
  read(n, m);
  var cnt: dynamic = 0;
  {
    var i: dynamic = ((2 * n) - 1);
    while ((i > 1))
    {
      if (((!a[i]) && (!a[(i - 1)])))
      {
        cnt += 1;
      }
      a[(i / 2)] = (a[i] && a[(i - 1)]);
      i -= 2;
    }
  }
  write(cnt, "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(ai);
    a[(n + ai)] = true;
  }

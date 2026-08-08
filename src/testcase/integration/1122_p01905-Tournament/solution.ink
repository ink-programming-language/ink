// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var pb = cpp_expression("#include <bi");

var a = cpp_array((1 << 10));

func main()
{
  var n: dynamic;
  var m: dynamic;
  var ai: dynamic;
  read(n, m);
  var cnt = 0;
  {
    var i = ((2 * n) - 1);
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

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(ai);
    a[(n + ai)] = true;
  }

// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var ri: dynamic = cpp_expression("#include");

var mk: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include");

var eb: dynamic = cpp_expression("#include <bi");

var is: dynamic = cpp_expression("#inclu");

var es: dynamic = cpp_expression("#incl");

var N: dynamic = 200010;

func read() -> dynamic
{
  var s: dynamic = 0;
  var w: dynamic = 1;
  var char: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      w = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    s = ((((s << 3)) + ((s << 1))) + ((ch ^ 48)));
    ch = getchar();
  }
  return (s * w);
}

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var Ans: dynamic = cpp_uninitialized();

var g: dynamic = cpp_array(N);

func main() -> dynamic
{
  n = read();
  {
    var int_cpp: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var int_cpp: dynamic = 1;
    while ((i <= n))
    {
      g[a[i]].eb(i);
      i += 1;
    }
  }
  {
    var int_cpp: dynamic = 1;
    while ((i <= n))
    {
      g[a[i]].eb((n + 1));
      i += 1;
    }
  }
  g[0].eb((n + 1));
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  p = cpp_assign(q, "=", 0);
  {
    var int_cpp: dynamic = 1;
    while ((i <= n))
    {
      if (((a[i] != p) && (a[i] != q)))
      {
        Ans += 1;
        var np: dynamic = (*lower_bound(g[p].begin(), g[p].end(), i));
        var nq: dynamic = (*lower_bound(g[q].begin(), g[q].end(), i));
        if ((np < nq))
        {
          q = a[i];
        } else
        {
          p = a[i];
        }
      } else
      {
        if ((a[i] == p))
        {
          p = a[i];
        } else if ((a[i] == q))
        {
          q = a[i];
        }
      }
      i += 1;
    }
  }
  printf("%lld\n", Ans);
  return 0;
}

// Translated from solution.cpp.

var int_cpp = dynamic;

var ri = cpp_expression("#include");

var mk = cpp_expression("#include");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var pb = cpp_expression("#include");

var eb = cpp_expression("#include <bi");

var is = cpp_expression("#inclu");

var es = cpp_expression("#incl");

var N = 200010;

func read()
{
  var s = 0;
  var w = 1;
  var char = getchar();
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

var n: dynamic;

var a = cpp_array(N);

var Ans: dynamic;

var g = cpp_array(N);

func main()
{
  n = read();
  {
    var int_cpp = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var int_cpp = 1;
    while ((i <= n))
    {
      g[a[i]].eb(i);
      i += 1;
    }
  }
  {
    var int_cpp = 1;
    while ((i <= n))
    {
      g[a[i]].eb((n + 1));
      i += 1;
    }
  }
  g[0].eb((n + 1));
  var p: dynamic;
  var q: dynamic;
  p = cpp_assign(q, "=", 0);
  {
    var int_cpp = 1;
    while ((i <= n))
    {
      if (((a[i] != p) && (a[i] != q)))
      {
        Ans += 1;
        var np = (*lower_bound(g[p].begin(), g[p].end(), i));
        var nq = (*lower_bound(g[q].begin(), g[q].end(), i));
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

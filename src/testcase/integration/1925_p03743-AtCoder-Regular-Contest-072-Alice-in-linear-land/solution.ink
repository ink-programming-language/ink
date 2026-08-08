// Translated from solution.cpp.

func yabs(x: dynamic)
{
  return cpp_expression("#include <bits/std");
}

var N = (5e5 + 10);

var Inf = (1e9 + 10);

var n: dynamic;

var D: dynamic;

var Q: dynamic;

var a = cpp_array(N);

var now = cpp_array(N);

var suf = cpp_array(N);

func Init()
{
  scanf("%d%d", (&n), (&D));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
}

func Solve()
{
  now[0] = D;
  {
    var i = 1;
    while ((i <= n))
    {
      now[i] = min(now[(i - 1)], yabs((now[(i - 1)] - a[i])));
      i += 1;
    }
  }
  suf[(n + 1)] = 1;
  {
    var i = n;
    while ((i >= 1))
    {
      if ((yabs((suf[(i + 1)] - a[i])) >= suf[(i + 1)]))
      {
        suf[i] = suf[(i + 1)];
      } else
      {
        suf[i] = min((suf[(i + 1)] + a[i]), Inf);
      }
      i -= 1;
    }
  }
  scanf("%d", (&Q));
  var x: dynamic;
  {
    var i = 1;
    while ((i <= Q))
    {
      scanf("%d", (&x));
      if ((suf[(x + 1)] > now[(x - 1)]))
      {
        printf("NO\n");
      } else
      {
        printf("YES\n");
      }
      i += 1;
    }
  }
}

func main()
{
  Init();
  Solve();
  return 0;
}

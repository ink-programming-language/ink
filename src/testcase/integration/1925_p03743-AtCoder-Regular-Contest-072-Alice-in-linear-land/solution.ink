// Translated from solution.cpp.

func yabs(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

var N: dynamic = (5e5 + 10);

var Inf: dynamic = (1e9 + 10);

var n: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var now: dynamic = cpp_array(N);

var suf: dynamic = cpp_array(N);

func Init() -> dynamic
{
  scanf("%d%d", (&n), (&D));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
}

func Solve() -> dynamic
{
  now[0] = D;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      now[i] = min(now[(i - 1)], yabs((now[(i - 1)] - a[i])));
      i += 1;
    }
  }
  suf[(n + 1)] = 1;
  {
    var i: dynamic = n;
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
  var x: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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

func main() -> dynamic
{
  Init();
  Solve();
  return 0;
}

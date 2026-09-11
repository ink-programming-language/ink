// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  var a: dynamic = cpp_construct((n + 2));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  var f: dynamic = cpp_construct((n + 2));
  f[1] = a[1];
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      f[i] = (a[i] - a[(i - 1)]);
      i += 1;
    }
  }
  var sum: dynamic = a[1];
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      if ((f[i] > 0))
      {
        sum += f[i];
      }
      i += 1;
    }
  }
  var ans: dynamic = cpp_uninitialized();
  if ((sum >= 0))
  {
    ans = ((sum / 2) + (sum % 2));
  } else
  {
    ans = (sum / 2);
  }
  printf("%lld\n", ans);
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    scanf("%d%d%lld", (&l), (&r), (&x));
    var tl: dynamic = f[l];
    var tr: dynamic = f[(r + 1)];
    if ((x != 0))
    {
      f[l] += x;
      f[(r + 1)] -= x;
      if ((x > 0))
      {
        if ((l == 1))
        {
          sum += x;
        }
        if ((((tl <= 0) && (f[l] > 0)) && (l != 1)))
        {
          sum += f[l];
        } else if (((tl > 0) && (l != 1)))
        {
          sum += x;
        }
        if (((r + 1) <= n))
        {
          if ((tr > 0))
          {
            sum -= min(x, tr);
          }
        }
      } else
      {
        if ((l == 1))
        {
          sum += x;
        }
        if (((tl > 0) && (l != 1)))
        {
          sum -= min(tl, (-x));
        }
        if (((r + 1) <= n))
        {
          if (((tr <= 0) && (f[(r + 1)] > 0)))
          {
            sum += f[(r + 1)];
          } else if ((tr > 0))
          {
            sum -= x;
          }
        }
      }
    }
    if ((sum >= 0))
    {
      ans = ((sum / 2) + (sum % 2));
    } else
    {
      ans = (sum / 2);
    }
    printf("%lld\n", ans);
  }
  return 0;
}

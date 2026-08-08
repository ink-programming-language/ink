// Translated from solution.cpp.

var md = 998244353;

var N = 200010;

var M = 3010;

func pow_mod(a: dynamic, b: dynamic)
{
  var ret = 1;
  while (b)
  {
    if ((b & 1))
    {
      ret = (((1 * ret) * a) % md);
    }
    b >>= 1;
    a = (((1 * a) * a) % md);
  }
  return ret;
}

var inv = cpp_array((M << 1));

var f = cpp_array(M, M);

var g = cpp_array(M, M);

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d%d", (&n), (&m));
  var w = cpp_construct((n + 1));
  var a = cpp_construct((n + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var SA = 0;
  var SB = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&w[i]));
      if ((a[i] == 0))
      {
        SB += w[i];
      } else
      {
        SA += w[i];
      }
      i += 1;
    }
  }
  var inv = cpp_construct((M << 1));
  {
    var delta = (-m);
    while ((delta <= m))
    {
      if ((((SA + SB) + delta) > 0))
      {
        inv[(delta + m)] = pow_mod(((SA + SB) + delta), (md - 2));
      }
      delta += 1;
    }
  }
  {
    var i = m;
    while ((i >= 0))
    {
      f[i][(m - i)] = cpp_assign(g[i][(m - i)], "=", 1);
      {
        var j = min(SB, ((m - i) - 1));
        while ((j >= 0))
        {
          f[i][j] += (((((1 * (((SA + i) + 1))) * f[(i + 1)][j]) % md) * inv[((i - j) + m)]) % md);
          f[i][j] %= md;
          f[i][j] += (((((1 * ((SB - j))) * f[i][(j + 1)]) % md) * inv[((i - j) + m)]) % md);
          f[i][j] %= md;
          g[i][j] += (((((1 * (((SB - j) - 1))) * g[i][(j + 1)]) % md) * inv[((i - j) + m)]) % md);
          g[i][j] %= md;
          g[i][j] += (((((1 * ((SA + i))) * g[(i + 1)][j]) % md) * inv[((i - j) + m)]) % md);
          g[i][j] %= md;
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i] == 1))
      {
        printf("%d\n", (((1 * w[i]) * f[0][0]) % md));
      } else
      {
        printf("%d\n", (((1 * w[i]) * g[0][0]) % md));
      }
      i += 1;
    }
  }
  return 0;
}

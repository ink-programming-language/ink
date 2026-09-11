// Translated from solution.cpp.

var md: dynamic = 998244353;

var N: dynamic = 200010;

var M: dynamic = 3010;

func pow_mod(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = 1;
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

var inv: dynamic = cpp_array((M << 1));

var f: dynamic = cpp_array(M, M);

var g: dynamic = cpp_array(M, M);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  var w: dynamic = cpp_construct((n + 1));
  var a: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var SA: dynamic = 0;
  var SB: dynamic = 0;
  {
    var i: dynamic = 1;
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
  var inv: dynamic = cpp_construct((M << 1));
  {
    var delta: dynamic = (-m);
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
    var i: dynamic = m;
    while ((i >= 0))
    {
      f[i][(m - i)] = cpp_assign(g[i][(m - i)], "=", 1);
      {
        var j: dynamic = min(SB, ((m - i) - 1));
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
    var i: dynamic = 1;
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

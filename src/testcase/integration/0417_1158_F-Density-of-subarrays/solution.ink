// Translated from solution.cpp.

var mod = 998244353;

var n: dynamic;

var c: dynamic;

var m: dynamic;

var a = cpp_array(3010);

func ksm(x: dynamic, y: dynamic = (mod - 2))
{
  var z = 1;
  {
    while (y)
    {
      if ((y & 1))
      {
        z = (((1 * z) * x) % mod);
      }
      y >>= 1;
      x = (((1 * x) * x) % mod);
    }
  }
  return z;
}

func ADD(x: dynamic, y: dynamic)
{
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
}

func SUM(x: dynamic, y: dynamic)
{
  if (((x + y) >= mod))
  {
    return ((x + y) - mod);
  }
  return (x + y);
}

var g = cpp_array(3010, 3010);

var cnt = cpp_array(3010);

var pov = cpp_array(3010);

var vop = cpp_array(3010);

var s = cpp_array(3010, 3010);

func solve()
{
  pov[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      pov[i] = (((pov[(i - 1)] << 1)) % mod);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      pov[i] = ((((pov[i] - 1) + mod)) % mod);
      vop[i] = ksm(pov[i]);
      i += 1;
    }
  }
  {
    var l = 1;
    while ((l <= n))
    {
      memset(cnt, 0, cpp_sizeof((cnt)));
      var ways = 1;
      var nil = (c - 1);
      {
        var r = (l + 1);
        while ((r <= n))
        {
          cnt[a[r]] += 1;
          if ((a[r] != a[l]))
          {
            if ((cnt[a[r]] == 1))
            {
              nil -= 1;
            } else
            {
              ways = (((1 * ways) * vop[(cnt[a[r]] - 1)]) % mod);
            }
            if ((!nil))
            {
              g[l][r] = ways;
            }
            ways = (((1 * ways) * pov[cnt[a[r]]]) % mod);
          } else
          {
            ways = (((ways << 1)) % mod);
          }
          r += 1;
        }
      }
      ways = 1;
      if (nil)
      {
        {
          var i = 1;
          while ((i <= c))
          {
            if (cnt[i])
            {
              ways = (((1 * ways) * ((pov[cnt[i]] + 1))) % mod);
            }
            i += 1;
          }
        }
      } else
      {
        {
          var i = 1;
          while ((i <= c))
          {
            ways = (((1 * ways) * ((pov[cnt[i]] + ((i == a[l]))))) % mod);
            i += 1;
          }
        }
        ways = SUM((pov[(n - l)] + 1), (mod - ways));
      }
      s[l][0] = ways;
      l += 1;
    }
  }
  s[(n + 1)][0] = 1;
  {
    var i = n;
    while (i)
    {
      {
        var j = 1;
        while ((j <= m))
        {
          {
            var k = ((i + c) - 1);
            while ((k <= n))
            {
              s[i][j] += (g[i][k] * s[(k + 1)][(j - 1)]);
              if ((!((k % 8))))
              {
                s[i][j] %= mod;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j <= m))
        {
          (cpp_assign(s[i][j], "+=", s[(i + 1)][j])) %= mod;
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      printf("%lld ", ((((s[1][i] + mod) - (!i))) % mod));
      i += 1;
    }
  }
  puts("");
}

var f = cpp_array((1 << 10), 3010, 2);

var lim: dynamic;

var res = cpp_array(3010);

func solve()
{
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] -= 1;
      i += 1;
    }
  }
  lim = (1 << c);
  f[0][0][0] = 1;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j <= m))
        {
          {
            var k = 0;
            while ((k < (lim - 1)))
            {
              f[(!((i & 1)))][j][k] = 0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j = 0;
        while ((j <= m))
        {
          {
            var k = 0;
            while ((k < (lim - 1)))
            {
              var K = (k | ((1 << a[(i + 1)])));
              if ((K == (lim - 1)))
              {
                K = 0;
              }
              (cpp_assign(f[(!((i & 1)))][j][k], "+=", f[(i & 1)][j][k])) %= mod;
              (cpp_assign(f[(!((i & 1)))][(j + (!K))][K], "+=", f[(i & 1)][j][k])) %= mod;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j = 0;
    while ((j <= n))
    {
      {
        var k = 0;
        while ((k < (lim - 1)))
        {
          (cpp_assign(res[j], "+=", f[(n & 1)][j][k])) %= mod;
          k += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      printf("%d ", ((((res[i] - (!i)) + mod)) % mod));
      i += 1;
    }
  }
  puts("");
}

func read(x: dynamic)
{
  x = 0;
  var c = getchar();
  while (((c > cpp_char("9")) || (c < cpp_char("0"))))
  {
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 3)) + ((x << 1))) + ((c ^ 48)));
    c = getchar();
  }
}

func main()
{
  read(n);
  read(c);
  m = (n / c);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  if ((c <= 10))
  {
    SUB2.solve();
  } else
  {
    SUB1.solve();
  }
  return 0;
}

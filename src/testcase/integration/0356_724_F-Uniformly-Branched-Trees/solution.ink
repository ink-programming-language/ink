// Translated from solution.cpp.

var fac = cpp_array(1010);

var inv = cpp_array(1010);

var mod: dynamic;

func ksm(a: dynamic, b: dynamic = (mod - 2))
{
  var r = 1;
  {
    while (b)
    {
      if ((b & 1))
      {
        r = (((1 * r) * a) % mod);
      }
      a = (((1 * a) * a) % mod);
      b >>= 1;
    }
  }
  return r;
}

func C(a: dynamic, b: dynamic)
{
  var r = inv[b];
  {
    b -= 1;
    while ((b >= 0))
    {
      r = (((1 * r) * ((a - b))) % mod);
      b -= 1;
    }
  }
  return r;
}

var f = cpp_array(1010, 12, 1010);

func main()
{
  var n: dynamic;
  var d: dynamic;
  scanf("%d%d%d", (&n), (&d), (&mod));
  if ((n <= 2))
  {
    puts("1");
    return 0;
  }
  fac[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      i += 1;
    }
  }
  inv[n] = ksm(fac[n]);
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      inv[i] = (((1 * inv[(i + 1)]) * ((i + 1))) % mod);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      f[1][0][i] = 1;
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= min(d, (i - 1))))
        {
          {
            var k = 1;
            while ((k <= n))
            {
              f[i][j][k] = f[i][j][(k - 1)];
              {
                var t = 1;
                while ((((t * k) <= i) && (t <= j)))
                {
                  f[i][j][k] = (((f[i][j][k] + (((1 * f[(i - (t * k))][(j - t)][(k - 1)]) * (C(((f[k][if ((k == 1)) 0 else (d - 1)][(k - 1)] + t) - 1), t))) % mod))) % mod);
                  t += 1;
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ((((f[n][d][(n / 2)] - (if (((n & 1))) 0 else C(f[(n / 2)][(d - 1)][((n / 2) - 1)], 2))) + mod)) % mod));
  return 0;
}

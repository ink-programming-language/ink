// Translated from solution.cpp.

var fac: dynamic = cpp_array(1010);

var inv: dynamic = cpp_array(1010);

var mod: dynamic = cpp_uninitialized();

func ksm(a: dynamic, b: dynamic = (mod - 2)) -> dynamic
{
  var r: dynamic = 1;
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

func C(a: dynamic, b: dynamic) -> dynamic
{
  var r: dynamic = inv[b];
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

var f: dynamic = cpp_array(1010, 12, 1010);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&d), (&mod));
  if ((n <= 2))
  {
    puts("1");
    return 0;
  }
  fac[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      i += 1;
    }
  }
  inv[n] = ksm(fac[n]);
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      inv[i] = (((1 * inv[(i + 1)]) * ((i + 1))) % mod);
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      f[1][0][i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= min(d, (i - 1))))
        {
          {
            var k: dynamic = 1;
            while ((k <= n))
            {
              f[i][j][k] = f[i][j][(k - 1)];
              {
                var t: dynamic = 1;
                while ((((t * k) <= i) && (t <= j)))
                {
                  f[i][j][k] = (((f[i][j][k] + (((1 * f[(i - (t * k))][(j - t)][(k - 1)]) * (C(((f[k][ ((k == 1)) ? 0 : (d - 1)][(k - 1)] + t) - 1), t))) % mod))) % mod);
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
  printf("%d\n", ((((f[n][d][(n / 2)] - ( (((n & 1))) ? 0 : C(f[(n / 2)][(d - 1)][((n / 2) - 1)], 2))) + mod)) % mod));
  return 0;
}

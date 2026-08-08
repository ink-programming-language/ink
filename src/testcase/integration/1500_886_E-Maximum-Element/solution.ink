// Translated from solution.cpp.

var N = (1e6 + 10);

var mod = (1e9 + 7);

var n: dynamic;

var k: dynamic;

var f = cpp_array(N);

var sum = cpp_array(N);

var fac = cpp_array(N);

var ifac = cpp_array(N);

func power(a: dynamic, b: dynamic)
{
  var ret = 1;
  while (b)
  {
    if ((b & 1))
    {
      ret = (((1 * ret) * a) % mod);
    }
    a = (((1 * a) * a) % mod);
    b >>= 1;
  }
  return ret;
}

func main()
{
  scanf("%d%d", (&n), (&k));
  fac[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      i += 1;
    }
  }
  ifac[0] = 1;
  ifac[n] = power(fac[n], (mod - 2));
  {
    var i = (n - 1);
    while ((i >= 1))
    {
      ifac[i] = (((1 * ifac[(i + 1)]) * ((i + 1))) % mod);
      i -= 1;
    }
  }
  f[0] = cpp_assign(sum[0], "=", 1);
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i >= (k + 1)))
      {
        f[i] = (((1 * (((((sum[(i - 1)] + mod) - sum[((i - k) - 1)])) % mod))) * fac[(i - 1)]) % mod);
      } else
      {
        f[i] = (((1 * sum[(i - 1)]) * fac[(i - 1)]) % mod);
      }
      sum[i] = (((sum[(i - 1)] + (((1 * f[i]) * ifac[i]) % mod))) % mod);
      i += 1;
    }
  }
  printf("%d\n", ((((fac[n] + mod) - (((1 * fac[(n - 1)]) * sum[(n - 1)]) % mod))) % mod));
  return 0;
}

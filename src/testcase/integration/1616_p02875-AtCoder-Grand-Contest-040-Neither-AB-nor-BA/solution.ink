// Translated from solution.cpp.

var SIZEN = 10000010;

var mod = 998244353;

var pw = cpp_array(SIZEN);

var fac = cpp_array(SIZEN);

var inv = cpp_array(SIZEN);

var N: dynamic;

func qpow(x: dynamic, len: dynamic)
{
  var ret = 1;
  {
    while (len)
    {
      if ((len & 1))
      {
        ret = ((ret * x) % mod);
      }
      x = ((x * x) % mod);
      len >>= 1;
    }
  }
  return ret;
}

func C(n: dynamic, m: dynamic)
{
  if ((n < m))
  {
    return 0;
  }
  return ((((fac[n] * inv[m]) % mod) * inv[(n - m)]) % mod);
}

func main()
{
  scanf("%d", (&N));
  {
    var i = 0;
    while ((i <= N))
    {
      pw[i] = (if ((i == 0)) 1 else ((pw[(i - 1)] * 2) % mod));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= N))
    {
      fac[i] = (if ((i == 0)) 1 else ((fac[(i - 1)] * i) % mod));
      i += 1;
    }
  }
  {
    var i = N;
    while ((i >= 0))
    {
      inv[i] = (if ((i == N)) qpow(fac[N], (mod - 2)) else ((inv[(i + 1)] * ((i + 1))) % mod));
      i -= 1;
    }
  }
  var ans = qpow(3, N);
  {
    var i = ((N / 2) + 1);
    while ((i <= N))
    {
      ans = (((ans - (((2 * C(N, i)) * pw[(N - i)]) % mod))) % mod);
      i += 1;
    }
  }
  printf("%lld", (((ans + mod)) % mod));
  return 0;
}

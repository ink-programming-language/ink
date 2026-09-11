// Translated from solution.cpp.

var SIZEN: dynamic = 10000010;

var mod: dynamic = 998244353;

var pw: dynamic = cpp_array(SIZEN);

var fac: dynamic = cpp_array(SIZEN);

var inv: dynamic = cpp_array(SIZEN);

var N: dynamic = cpp_uninitialized();

func qpow(x: dynamic, len: dynamic) -> dynamic
{
  var ret: dynamic = 1;
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

func C(n: dynamic, m: dynamic) -> dynamic
{
  if ((n < m))
  {
    return 0;
  }
  return ((((fac[n] * inv[m]) % mod) * inv[(n - m)]) % mod);
}

func main() -> dynamic
{
  scanf("%d", (&N));
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      pw[i] = ( ((i == 0)) ? 1 : ((pw[(i - 1)] * 2) % mod));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      fac[i] = ( ((i == 0)) ? 1 : ((fac[(i - 1)] * i) % mod));
      i += 1;
    }
  }
  {
    var i: dynamic = N;
    while ((i >= 0))
    {
      inv[i] = ( ((i == N)) ? qpow(fac[N], (mod - 2)) : ((inv[(i + 1)] * ((i + 1))) % mod));
      i -= 1;
    }
  }
  var ans: dynamic = qpow(3, N);
  {
    var i: dynamic = ((N / 2) + 1);
    while ((i <= N))
    {
      ans = (((ans - (((2 * C(N, i)) * pw[(N - i)]) % mod))) % mod);
      i += 1;
    }
  }
  printf("%lld", (((ans + mod)) % mod));
  return 0;
}

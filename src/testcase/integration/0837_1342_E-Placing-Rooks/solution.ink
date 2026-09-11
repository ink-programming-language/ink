// Translated from solution.cpp.

var N: dynamic = (2e5 + 5);

var MOD: dynamic = 998244353;

var INF: dynamic = 0x3f3f3f3f;

var INF_LL: dynamic = 0x3f3f3f3f3f3f3f3f;

func QPow(bas: dynamic, t: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  bas %= MOD;
  while (t)
  {
    if ((t & 1))
    {
      ret = ((ret * bas) % MOD);
    }
    bas = ((bas * bas) % MOD);
    t >>= 1;
  }
  return ret;
}

func Inv(x: dynamic) -> dynamic
{
  return QPow(x, (MOD - 2));
}

var fac: dynamic = cpp_array(N);

var ifac: dynamic = cpp_array(N);

func Init() -> dynamic
{
  fac[0] = cpp_assign(ifac[0], "=", 1);
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      fac[i] = ((fac[(i - 1)] * i) % MOD);
      ifac[i] = Inv(fac[i]);
      i += 1;
    }
  }
}

func C(n: dynamic, a: dynamic) -> dynamic
{
  if ((((n < 0) || (a < 0)) || ((n - a) < 0)))
  {
    return 0;
  } else
  {
    return ((((fac[n] * ifac[a]) % MOD) * ifac[(n - a)]) % MOD);
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  Init();
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var l: dynamic = (n - k);
  if ((l < 0))
  {
    write(0, "\n");
    return 0;
  }
  if ((k == 0))
  {
    write(fac[n], "\n");
    return 0;
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= l))
    {
      var tmp: dynamic = (((QPow(-1, i) * C(l, i)) % MOD) * QPow((l - i), n));
      ans = (((ans + tmp)) % MOD);
      i += 1;
    }
  }
  ans = (((2 * ans) * C(n, l)) % MOD);
  ans = (((ans + MOD)) % MOD);
  write(ans, "\n");
  return 0;
}

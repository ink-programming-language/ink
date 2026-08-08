// Translated from solution.cpp.

var N = (2e5 + 5);

var MOD = 998244353;

var INF = 0x3f3f3f3f;

var INF_LL = 0x3f3f3f3f3f3f3f3f;

func QPow(bas: dynamic, t: dynamic)
{
  var ret = 1;
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

func Inv(x: dynamic)
{
  return QPow(x, (MOD - 2));
}

var fac = cpp_array(N);

var ifac = cpp_array(N);

func Init()
{
  fac[0] = cpp_assign(ifac[0], "=", 1);
  {
    var i = 1;
    while ((i < N))
    {
      fac[i] = ((fac[(i - 1)] * i) % MOD);
      ifac[i] = Inv(fac[i]);
      i += 1;
    }
  }
}

func C(n: dynamic, a: dynamic)
{
  if ((((n < 0) || (a < 0)) || ((n - a) < 0)))
  {
    return 0;
  } else
  {
    return ((((fac[n] * ifac[a]) % MOD) * ifac[(n - a)]) % MOD);
  }
}

func main()
{
  ios.sync_with_stdio(0);
  Init();
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var l = (n - k);
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
  var ans = 0;
  {
    var i = 0;
    while ((i <= l))
    {
      var tmp = (((QPow(-1, i) * C(l, i)) % MOD) * QPow((l - i), n));
      ans = (((ans + tmp)) % MOD);
      i += 1;
    }
  }
  ans = (((2 * ans) * C(n, l)) % MOD);
  ans = (((ans + MOD)) % MOD);
  write(ans, "\n");
  return 0;
}

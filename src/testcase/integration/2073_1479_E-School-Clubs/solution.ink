// Translated from solution.cpp.

var N = 1010;

var MOD = 998244353;

func bigMod(a: dynamic, e: dynamic)
{
  if ((e == -1))
  {
    e = (MOD - 2);
  }
  var ret = 1;
  while (e)
  {
    if ((e & 1))
    {
      ret = ((ret * a) % MOD);
    }
    a = ((a * a) % MOD);
    e >>= 1;
  }
  return ret;
}

var m: dynamic;

var n: dynamic;

var a = cpp_array(N);

func main()
{
  read(m);
  {
    var i = 1;
    while ((i <= m))
    {
      read(a[i]);
      n += a[i];
      i += 1;
    }
  }
  sort((a + 1), ((a + m) + 1));
  var ans = 0;
  var up = 1;
  var down = 1;
  var at = 0;
  var p = (n + n);
  var q = (n + 1);
  {
    var i = 1;
    while ((i <= m))
    {
      while ((at < a[i]))
      {
        at += 1;
        p -= 1;
        q -= 1;
        up = ((up * p) % MOD);
        down = ((down * q) % MOD);
      }
      ans += ((up * bigMod(down, -1)) % MOD);
      i += 1;
    }
  }
  while ((at < n))
  {
    at += 1;
    p -= 1;
    q -= 1;
    up = ((up * p) % MOD);
    down = ((down * q) % MOD);
  }
  ans -= ((up * bigMod(down, -1)) % MOD);
  ans -= (m - 1);
  ans %= MOD;
  ans *= ((n * bigMod((n - 1), -1)) % MOD);
  ans %= MOD;
  ans *= -2;
  ans %= MOD;
  ans += MOD;
  ans %= MOD;
  write(ans, cpp_char("\n"));
  return 0;
}

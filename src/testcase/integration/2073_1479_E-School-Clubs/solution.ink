// Translated from solution.cpp.

var N: dynamic = 1010;

var MOD: dynamic = 998244353;

func bigMod(a: dynamic, e: dynamic) -> dynamic
{
  if ((e == -1))
  {
    e = (MOD - 2);
  }
  var ret: dynamic = 1;
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

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  read(m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(a[i]);
      n += a[i];
      i += 1;
    }
  }
  sort((a + 1), ((a + m) + 1));
  var ans: dynamic = 0;
  var up: dynamic = 1;
  var down: dynamic = 1;
  var at: dynamic = 0;
  var p: dynamic = (n + n);
  var q: dynamic = (n + 1);
  {
    var i: dynamic = 1;
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

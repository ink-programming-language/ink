// Translated from solution.cpp.

var MOD: dynamic = 998244353;

var INF32: dynamic = (1 << 30);

var INF64: dynamic = (1 << 60);

var pi: dynamic = acos(-1);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((!b)) ? a : gcd(b, (a % b)));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) / gcd(a, b));
}

func modpow(b: dynamic, i: dynamic) -> dynamic
{
  var s: dynamic = 1;
  while (i)
  {
    if ((i % 2))
    {
      s = (((s * b)) % MOD);
    }
    b = (((b * b)) % MOD);
    i /= 2;
  }
  return s;
}

func inv(a: dynamic) -> dynamic
{
  return modpow(a, (MOD - 2));
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  return (((a + b)) % MOD);
}

func sub(a: dynamic, b: dynamic) -> dynamic
{
  return ((((a - b) + MOD)) % MOD);
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) % MOD);
}

func nCr(n: dynamic, r: dynamic) -> dynamic
{
  var m1: dynamic = 1;
  var m2: dynamic = 1;
  r = min(r, (n - r));
  {
    var i: dynamic = 0;
    while ((i < r))
    {
      m1 = (((m1 * ((n - i)))) % MOD);
      m2 = (((m2 * ((r - i)))) % MOD);
      i += 1;
    }
  }
  return (((m1 * inv(m2))) % MOD);
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var cnt: dynamic = 0;
  var sum: dynamic = 0;
  var mn: dynamic = 1000;
  {
    var i: dynamic = 0;
    while ((i < ((2 * n) - 1)))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      if ((x < 0))
      {
        cnt += 1;
      }
      sum += abs(x);
      mn = min(mn, abs(x));
      i += 1;
    }
  }
  if ((((n % 2) == 0) && (cnt % 2)))
  {
    write((sum - (2 * mn)));
  } else
  {
    write(sum);
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  solve();
  return 0;
}

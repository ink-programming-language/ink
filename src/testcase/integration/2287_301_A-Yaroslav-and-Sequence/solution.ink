// Translated from solution.cpp.

var MOD = 998244353;

var INF32 = (1 << 30);

var INF64 = (1 << 60);

var pi = acos(-1);

func gcd(a: dynamic, b: dynamic)
{
  return (if ((!b)) a else gcd(b, (a % b)));
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a * b) / gcd(a, b));
}

func modpow(b: dynamic, i: dynamic)
{
  var s = 1;
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

func inv(a: dynamic)
{
  return modpow(a, (MOD - 2));
}

func add(a: dynamic, b: dynamic)
{
  return (((a + b)) % MOD);
}

func sub(a: dynamic, b: dynamic)
{
  return ((((a - b) + MOD)) % MOD);
}

func mul(a: dynamic, b: dynamic)
{
  return ((a * b) % MOD);
}

func nCr(n: dynamic, r: dynamic)
{
  var m1 = 1;
  var m2 = 1;
  r = min(r, (n - r));
  {
    var i = 0;
    while ((i < r))
    {
      m1 = (((m1 * ((n - i)))) % MOD);
      m2 = (((m2 * ((r - i)))) % MOD);
      i += 1;
    }
  }
  return (((m1 * inv(m2))) % MOD);
}

func solve()
{
  var n: dynamic;
  read(n);
  var cnt = 0;
  var sum = 0;
  var mn = 1000;
  {
    var i = 0;
    while ((i < ((2 * n) - 1)))
    {
      var x: dynamic;
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  solve();
  return 0;
}

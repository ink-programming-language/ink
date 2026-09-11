// Translated from solution.cpp.

var SF: dynamic = cpp_expression("#incl");

var PF: dynamic = cpp_expression("#inclu");

func power(x: dynamic, num: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (num)
  {
    if ((num & 1))
    {
      res = (res * x);
      res %= mod;
    }
    x *= x;
    x %= mod;
    num >>= 1;
  }
  return res;
}

func phi(n: dynamic) -> dynamic
{
  var res: dynamic = n;
  var t: dynamic = n;
  {
    var i: dynamic = 2;
    while (((i * i) <= t))
    {
      if (((t % i) == 0))
      {
        res -= (res / i);
        while (((t % i) == 0))
        {
          t /= i;
        }
      }
      i += 1;
    }
  }
  if ((t > 1))
  {
    res -= (res / t);
  }
  return res;
}

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  if ((y == 0))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var g: dynamic = cpp_uninitialized();
  g = exgcd(b, (a % b), y, x);
  y -= ((a / b) * x);
  return g;
}

func solve(a: dynamic, mod: dynamic) -> dynamic
{
  if ((mod == 1))
  {
    return 1;
  }
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var pm: dynamic = phi(mod);
  var d: dynamic = gcd(mod, pm);
  var t: dynamic = solve(a, d);
  var at: dynamic = power(a, t, mod);
  exgcd((mod / d), (pm / d), x, y);
  x = ((x * (((((((t - at)) % pm) + pm)) % pm))) / d);
  x = ((((((x % ((pm / d)))) + (pm / d))) % ((pm / d))) + (pm / d));
  return ((x * mod) + at);
}

var a: dynamic = cpp_uninitialized();

var mod: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  SF("%d", (&q));
  while (cpp_update(q, "--"))
  {
    SF("%lld%lld", (&a), (&mod));
    PF("%lld\n", solve(a, mod));
  }
}

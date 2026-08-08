// Translated from solution.cpp.

var SF = cpp_expression("#incl");

var PF = cpp_expression("#inclu");

func power(x: dynamic, num: dynamic, mod: dynamic)
{
  var res = 1;
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

func phi(n: dynamic)
{
  var res = n;
  var t = n;
  {
    var i = 2;
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

func gcd(x: dynamic, y: dynamic)
{
  if ((y == 0))
  {
    return x;
  }
  return gcd(y, (x % y));
}

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var g: dynamic;
  g = exgcd(b, (a % b), y, x);
  y -= ((a / b) * x);
  return g;
}

func solve(a: dynamic, mod: dynamic)
{
  if ((mod == 1))
  {
    return 1;
  }
  var x: dynamic;
  var y: dynamic;
  var pm = phi(mod);
  var d = gcd(mod, pm);
  var t = solve(a, d);
  var at = power(a, t, mod);
  exgcd((mod / d), (pm / d), x, y);
  x = ((x * (((((((t - at)) % pm) + pm)) % pm))) / d);
  x = ((((((x % ((pm / d)))) + (pm / d))) % ((pm / d))) + (pm / d));
  return ((x * mod) + at);
}

var a: dynamic;

var mod: dynamic;

func main()
{
  var q: dynamic;
  SF("%d", (&q));
  while (cpp_update(q, "--"))
  {
    SF("%lld%lld", (&a), (&mod));
    PF("%lld\n", solve(a, mod));
  }
}

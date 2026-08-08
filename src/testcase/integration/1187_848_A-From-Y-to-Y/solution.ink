// Translated from solution.cpp.

class custom_hash
{
  func splitmix64(x: dynamic)
  {
      x += 0x9e3779b97f4a7c15;
      x = (((x ^ ((x >> 30)))) * 0xbf58476d1ce4e5b9);
      x = (((x ^ ((x >> 27)))) * 0x94d049bb133111eb);
      return (x ^ ((x >> 31)));
    }
  func operator_call(x: dynamic)
  {
      var FIXED_RANDOM = chrono.steady_clock.now().time_since_epoch().count();
      return splitmix64((x + FIXED_RANDOM));
    }
}

func isPrime(n: dynamic)
{
  if ((n <= 1))
  {
    return false;
  }
  if ((n <= 3))
  {
    return true;
  }
  if ((((n % 2) == 0) || ((n % 3) == 0)))
  {
    return false;
  }
  {
    var i = 5;
    while (((i * i) <= n))
    {
      if ((((n % i) == 0) || ((n % ((i + 2))) == 0)))
      {
        return false;
      }
      i = (i + 6);
    }
  }
  return true;
}

func nextPrime(N: dynamic)
{
  if ((N <= 1))
  {
    return 2;
  }
  var prime = N;
  var found = false;
  while ((!found))
  {
    prime += 1;
    if (isPrime(prime))
    {
      found = true;
    }
  }
  return prime;
}

func fact(n: dynamic)
{
  if ((n == 1))
  {
    return 1;
  }
  return (n * fact((n - 1)));
}

func cl(n: dynamic, d: dynamic)
{
  return ((((n + d) - 1)) / d);
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a * b)) / (gcd(a, b)));
}

func Pow(x: dynamic, n: dynamic)
{
  var ans = 1;
  while (n)
  {
    if ((n & 1))
    {
      ans = ((ans * x));
    }
    x = ((x * x));
    n >>= 1;
  }
  return ans;
}

func solve()
{
  var k: dynamic;
  var i: dynamic;
  read(k);
  if ((!k))
  {
    write(cpp_char("a"));
    return;
  }
  var v: dynamic;
  {
    i = 2000;
    while ((i >= 2))
    {
      while (((k - (((i * ((i - 1)))) / 2)) >= 0))
      {
        v.push_back(i);
        k = (k - (((i * ((i - 1)))) / 2));
      }
      i -= 1;
    }
  }
  var c = cpp_char("a");
  for (var x in v)
  {
    {
      i = 0;
      while ((i < x))
      {
        write(c);
        i += 1;
      }
    }
    c += 1;
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}

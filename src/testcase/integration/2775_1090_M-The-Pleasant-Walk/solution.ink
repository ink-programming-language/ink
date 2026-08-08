// Translated from solution.cpp.

var N = (1e5 + 5);

func pairsort(a: dynamic, b: dynamic, n: dynamic)
{
  var pairt = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      pairt[i].first = a[i];
      pairt[i].second = b[i];
      i += 1;
    }
  }
  sort(pairt, (pairt + n));
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = pairt[i].first;
      b[i] = pairt[i].second;
      i += 1;
    }
  }
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func isPrime(n: dynamic)
{
  if ((n < 2))
  {
    return 0;
  }
  if ((n < 4))
  {
    return 1;
  }
  if (cpp_binary(((n % 2) == 0), "or", ((n % 3) == 0)))
  {
    return 0;
  }
  {
    var i = 5;
    while (((i * i) <= n))
    {
      if (cpp_binary(((n % i) == 0), "or", ((n % ((i + 2))) == 0)))
      {
        return 0;
      }
      i += 6;
    }
  }
  return 1;
}

func C(n: dynamic, r: dynamic)
{
  if ((r > (n - r)))
  {
    r = (n - r);
  }
  var ans = 1;
  {
    var i = 1;
    while ((i <= r))
    {
      ans *= ((n - r) + i);
      ans /= i;
      i += 1;
    }
  }
  return ans;
}

var mod = (1e9 + 7);

func modexpo(x: dynamic, p: dynamic)
{
  var res = 1;
  x = (x % mod);
  while (p)
  {
    if ((p % 2))
    {
      res = (res * x);
    }
    p >>= 1;
    x = ((x * x) % mod);
    res %= mod;
  }
  return res;
}

var n: dynamic;

var k: dynamic;

var cnt = 1;

var a = cpp_array(100005);

var ans = 1;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      if ((a[i] == a[(i - 1)]))
      {
        cnt = 1;
      } else
      {
        cnt += 1;
      }
      ans = (if ((ans > cnt)) ans else cnt);
      i += 1;
    }
  }
  write(ans);
  return 0;
}

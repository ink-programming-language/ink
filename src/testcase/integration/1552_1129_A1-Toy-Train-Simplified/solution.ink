// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var prime: dynamic = cpp_array(1000001);

var spf: dynamic = cpp_array(10000001);

var f: dynamic = cpp_array(300005);

func pow1(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = 1;
  x = (x % mod);
  if ((x == 0))
  {
    return 0;
  }
  while ((y > 0))
  {
    if ((y & 1))
    {
      res = (((res * x)) % mod);
    }
    y = (y >> 1);
    x = (((x * x)) % mod);
  }
  return res;
}

func divide(n: dynamic) -> dynamic
{
  return pow1(n, (mod - 2));
}

func ncr(n: dynamic, r: dynamic) -> dynamic
{
  if ((n < r))
  {
    return 0;
  }
  return (((f[n] * ((((divide(f[r]) * divide(f[(n - r)]))) % mod)))) % mod);
}

func sieve() -> dynamic
{
  memset(prime, true, cpp_sizeof((prime)));
  {
    var i: dynamic = 2;
    while (((i * i) <= 1000000))
    {
      if (prime[i])
      {
        {
          var j: dynamic = (i * i);
          while ((j <= 1000000))
          {
            prime[j] = false;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  prime[0] = cpp_assign(prime[1], "=", false);
}

func fastsieve() -> dynamic
{
  spf[1] = 1;
  {
    var i: dynamic = 2;
    while ((i <= 1e7))
    {
      spf[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 4;
    while ((i <= 1e7))
    {
      spf[i] = 2;
      i += 2;
    }
  }
  {
    var i: dynamic = 3;
    while (((i * i) <= 1e7))
    {
      if ((spf[i] == i))
      {
        {
          var j: dynamic = (i * i);
          while ((j <= 1e7))
          {
            if ((spf[j] == j))
            {
              spf[j] = i;
            }
            j += i;
          }
        }
      }
      i += 1;
    }
  }
}

func factorize(n: dynamic) -> dynamic
{
  var count: dynamic = 0;
  var fac: dynamic = cpp_uninitialized();
  while ((!((n % 2))))
  {
    n >>= 1;
    count += 1;
  }
  if ((count % 2))
  {
    fac.push_back(2);
  }
  {
    var i: dynamic = 3;
    while ((i <= sqrt(n)))
    {
      count = 0;
      while (((n % i) == 0))
      {
        count += 1;
        n = (n / i);
      }
      if ((count % 2))
      {
        fac.push_back(i);
      }
      i += 2;
    }
  }
  if ((n > 2))
  {
    fac.push_back(n);
  }
  return fac;
}

func fastfactorize(n: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  var prev: dynamic = 0;
  var cnt: dynamic = 0;
  while ((n != 1))
  {
    if ((prev == spf[n]))
    {
      cnt += 1;
    } else
    {
      if ((cnt % 2))
      {
        v.push_back(prev);
      }
      cnt = 1;
      prev = spf[n];
    }
    n /= spf[n];
    if ((n == 1))
    {
      if ((cnt % 2))
      {
        v.push_back(prev);
      }
      cnt = 1;
      prev = spf[n];
    }
  }
  return v;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  read(n, m);
  var cnt: dynamic = cpp_array((n + 1));
  var val: dynamic = cpp_array((n + 1));
  memset(cnt, 0, cpp_sizeof((cnt)));
  {
    i = 0;
    while ((i <= n))
    {
      val[i] = 1e18;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      cnt[x] += 1;
      val[x] = min(val[x], ((((y - x) + n)) % n));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      var ans: dynamic = 0;
      {
        j = 1;
        while ((j <= n))
        {
          if ((!cnt[j]))
          {
            j += 1;
            continue;
          }
          ans = max(ans, ((((((j - i) + n)) % n) + (n * ((cnt[j] - 1)))) + val[j]));
          j += 1;
        }
      }
      write(ans, " ");
      i += 1;
    }
  }
  return 0;
}

// Translated from solution.cpp.

func power(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  if (((b % 2) == 0))
  {
    return ((power((((a * a)) % 998244353), (b / 2))) % 998244353);
  }
  return ((((a * (power((((a * a)) % 998244353), (b / 2)))) % 998244353)) % 998244353);
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func modInv(n: dynamic)
{
  return power(n, (1000000007 - 2));
}

func nCr(n: dynamic, r: dynamic, p: dynamic = 1000000007)
{
  if ((r == 0))
  {
    return 1;
  }
  var Fact = cpp_array((n + 1));
  Fact[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      Fact[i] = (((Fact[(i - 1)] * i)) % p);
      i += 1;
    }
  }
  return (((((((Fact[n] * modInv(Fact[r]))) % 1000000007)) * ((modInv(Fact[(n - r)]) % 1000000007)))) % 1000000007);
}

func main()
{
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var x: dynamic;
    var i: dynamic;
    var j: dynamic;
    read(n, x);
    var a = cpp_array(n);
    {
      i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    i = 0;
    var sum = 0;
    var ans = 0;
    var len = 0;
    while ((i < n))
    {
      sum = (sum + a[i]);
      if (((sum % x) != 0))
      {
        len = (i + 1);
      }
      i += 1;
    }
    ans = len;
    len = n;
    i = 0;
    while ((i < n))
    {
      sum = (sum - a[i]);
      len -= 1;
      i += 1;
      if (((sum % x) != 0))
      {
        ans = max(len, ans);
      }
    }
    if ((ans != 0))
    {
      write(ans, "\n");
    } else
    {
      write("-1\n");
    }
  }
}

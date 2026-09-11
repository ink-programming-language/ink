// Translated from solution.cpp.

var memo: dynamic = cpp_array(((1 << 21)));

func modexp(a: dynamic, n: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (n)
  {
    if ((n & 1))
    {
      res = (((((res % 1000000007)) * ((a % 1000000007)))) % 1000000007);
    }
    a = (((((a % 1000000007)) * ((a % 1000000007)))) % 1000000007);
    n >>= 1;
  }
  return res;
}

func main(argument_0: dynamic) -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var t: dynamic = cpp_uninitialized();
      read(t);
      memo[t] += 1;
      i += 1;
    }
  }
  {
    var p: dynamic = 0;
    while ((p < 21))
    {
      {
        var mask: dynamic = ((((1 << 21)) - 1));
        while ((mask >= 0))
        {
          if ((!((mask & ((1 << p))))))
          {
            memo[mask] += memo[(mask ^ ((1 << p)))];
          }
          mask -= 1;
        }
      }
      p += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var mask: dynamic = 0;
    while ((mask < ((1 << 21))))
    {
      var z: dynamic = builtin_popcount(mask);
      if ((z & 1))
      {
        ans = ((((((ans % 1000000007)) - ((modexp(2, memo[mask]) % 1000000007))) + 1000000007)) % 1000000007);
      } else
      {
        ans = (((((ans % 1000000007)) + ((modexp(2, memo[mask]) % 1000000007)))) % 1000000007);
      }
      mask += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

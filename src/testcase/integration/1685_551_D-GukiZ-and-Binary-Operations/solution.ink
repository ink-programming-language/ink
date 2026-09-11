// Translated from solution.cpp.

func FastIO() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
}

func modpow(a: dynamic, p: dynamic, mod: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  while (p)
  {
    if ((p & 1))
    {
      ret = (((ret * a)) % mod);
    }
    a = (((a * a)) % mod);
    p /= 2;
  }
  return ret;
}

func power(a: dynamic, p: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  while (p)
  {
    if ((p & 1))
    {
      ret = ((ret * a));
    }
    a = ((a * a));
    p /= 2;
  }
  return ret;
}

func fib(n: dynamic, mod: dynamic = LLONG_MAX) -> dynamic
{
  var fib: dynamic = [[1, 1], [1, 0]];
  var ret: dynamic = [[1, 0], [0, 1]];
  var tmp: dynamic = [[0, 0], [0, 0]];
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  while (n)
  {
    if ((n & 1))
    {
      memset(tmp, 0, cpp_sizeof((tmp)));
      {
        var i: dynamic = 0;
        while ((i < 2))
        {
          {
            var j: dynamic = 0;
            while ((j < 2))
            {
              {
                var k: dynamic = 0;
                while ((k < 2))
                {
                  tmp[i][j] = (((tmp[i][j] + (((ret[i][k] * fib[k][j])) % mod))) % mod);
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < 2))
        {
          {
            var j: dynamic = 0;
            while ((j < 2))
            {
              ret[i][j] = tmp[i][j];
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
    memset(tmp, 0, cpp_sizeof((tmp)));
    {
      var i: dynamic = 0;
      while ((i < 2))
      {
        {
          var j: dynamic = 0;
          while ((j < 2))
          {
            {
              var k: dynamic = 0;
              while ((k < 2))
              {
                tmp[i][j] = (((tmp[i][j] + (((fib[i][k] * fib[k][j])) % mod))) % mod);
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < 2))
      {
        {
          var j: dynamic = 0;
          while ((j < 2))
          {
            fib[i][j] = tmp[i][j];
            j += 1;
          }
        }
        i += 1;
      }
    }
    n /= 2;
  }
  return (ret[0][1]);
}

func main() -> dynamic
{
  FastIO();
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, k, l, m);
  var ans: dynamic = 1;
  var f: dynamic = fib((n + 2), m);
  var p: dynamic = ((((modpow(2, n, m) - f) + m)) % m);
  if (((l < 63) && (((1 << l)) <= k)))
  {
    write(0, cpp_char("\n"));
    return 0;
  }
  ans %= m;
  {
    var i: dynamic = 0;
    while ((i < l))
    {
      if ((((k >> i)) & 1))
      {
        ans = (((ans * p)) % m);
      } else
      {
        ans = (((ans * f)) % m);
      }
      i += 1;
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}

// Translated from solution.cpp.

var eps: dynamic = 1e-13;

var PI: dynamic = acos(-1);

var INF: dynamic = (cpp_cast(1e9) + 7);

var INFF: dynamic = cpp_cast(1e18);

var mod: dynamic = (cpp_cast(1e9) + 7);

var MXN: dynamic = (cpp_cast(1e2) + 7);

class Mat
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(MXN, MXN);
  func init(n: dynamic, m: dynamic) -> dynamic
  {
      n = n;
      m = m;
      {
        var i: dynamic = 1;
        while ((i < (n + 1)))
        {
          {
            var j: dynamic = 1;
            while ((j < (m + 1)))
            {
              a[i][j] = 0;
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  func operator_multiply(p2: dynamic) -> dynamic
  {
      var res: dynamic = cpp_uninitialized();
      res.init(n, p2.m);
      {
        var i: dynamic = 1;
        while ((i < (n + 1)))
        {
          {
            var j: dynamic = 1;
            while ((j < (m + 1)))
            {
              {
                var k: dynamic = 1;
                while ((k < (p2.m + 1)))
                {
                  res.a[i][k] = (((res.a[i][k] + (a[i][j] * p2.a[j][k]))) % mod);
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      return res;
    }
  func operator(p2: dynamic) -> dynamic
  {
      var t: dynamic = (p2 - 1);
      var res: dynamic = (*self);
      var x: dynamic = (*self);
      while (t)
      {
        if ((t & 1))
        {
          res = (res * x);
        }
        t >>= 1;
        x = (x * x);
      }
      return res;
    }
}

var b: dynamic = cpp_array(2);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%lld %d", (&n), (&m));
  if ((n <= m))
  {
    if ((n < m))
    {
      puts("1");
    } else
    {
      puts("2");
    }
    return 0;
  }
  b[0].init(m, m);
  {
    var i: dynamic = 1;
    while ((i < m))
    {
      b[0].a[i][(i + 1)] = 1;
      i += 1;
    }
  }
  b[0].a[m][1] = cpp_assign(b[0].a[m][m], "=", 1);
  b[1] = (b[0] ^ ((n - m)));
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      ans = (((ans + b[1].a[m][i])) % mod);
      i += 1;
    }
  }
  ans = (((ans + b[1].a[m][m])) % mod);
  printf("%lld\n", ans);
  return 0;
}

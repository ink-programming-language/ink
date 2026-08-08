// Translated from solution.cpp.

var eps = 1e-13;

var PI = acos(-1);

var INF = (cpp_cast(1e9) + 7);

var INFF = cpp_cast(1e18);

var mod = (cpp_cast(1e9) + 7);

var MXN = (cpp_cast(1e2) + 7);

class Mat
{
  var n: dynamic;
  var m: dynamic;
  var a: dynamic = cpp_array(MXN, MXN);
  func init(n: dynamic, m: dynamic)
  {
      n = n;
      m = m;
      {
        var i = 1;
        while ((i < (n + 1)))
        {
          {
            var j = 1;
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
  func operator_multiply(p2: dynamic)
  {
      var res: dynamic;
      res.init(n, p2.m);
      {
        var i = 1;
        while ((i < (n + 1)))
        {
          {
            var j = 1;
            while ((j < (m + 1)))
            {
              {
                var k = 1;
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
  func operator(p2: dynamic)
  {
      var t = (p2 - 1);
      var res = (*this);
      var x = (*this);
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

var b = cpp_array(2);

func main()
{
  var n: dynamic;
  var m: dynamic;
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
    var i = 1;
    while ((i < m))
    {
      b[0].a[i][(i + 1)] = 1;
      i += 1;
    }
  }
  b[0].a[m][1] = cpp_assign(b[0].a[m][m], "=", 1);
  b[1] = (b[0] ^ ((n - m)));
  var ans = 0;
  {
    var i = 1;
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

// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(2005);

var b = cpp_array(2005);

var n1: dynamic;

var n2: dynamic;

var f = cpp_array(13, 2005, 2);

var g = cpp_array(13, 2005, 2);

var c = cpp_array(2005, 2005);

var pw = cpp_array(2005);

var t: dynamic;

func calc(n: dynamic, m: dynamic)
{
  if ((m == 0))
  {
    return ((n == 0));
  }
  return ((pw[n] * c[((n + m) - 1)][(m - 1)]) % 998244353);
}

func main()
{
  pw[0] = 1;
  c[0][0] = 1;
  {
    var i = 1;
    while ((i <= 2000))
    {
      pw[i] = ((pw[(i - 1)] * i) % 998244353);
      c[i][0] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= 2000))
    {
      {
        var j = 1;
        while ((j <= 2000))
        {
          c[i][j] = (((c[(i - 1)][j] + c[(i - 1)][(j - 1)])) % 998244353);
          j += 1;
        }
      }
      i += 1;
    }
  }
  scanf("%lld", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%lld", (&n));
    n1 = cpp_assign(n2, "=", 0);
    {
      var i = 1;
      while ((i <= n))
      {
        var x: dynamic;
        var p = 0;
        scanf("%lld", (&x));
        {
          var j = x;
          while (j)
          {
            p ^= 1;
            j /= 10;
          }
        }
        if (p)
        {
          a[cpp_update(n1, "++")] = (x % 11);
        } else
        {
          b[cpp_update(n2, "++")] = (x % 11);
        }
        i += 1;
      }
    }
    memset(f, 0, cpp_sizeof(f));
    memset(g, 0, cpp_sizeof(g));
    f[0][0][0] = cpp_assign(g[0][0][0], "=", 1);
    {
      var i = 1;
      while ((i <= n1))
      {
        memset(f[(i & 1)], 0, cpp_sizeof(f[(i & 1)]));
        {
          var j = 0;
          while ((j <= i))
          {
            {
              var k = 0;
              while ((k < 11))
              {
                var p = ((((k - a[i]) + 11)) % 11);
                var q = (((k + a[i])) % 11);
                f[(i & 1)][j][k] = (((f[(i & 1)][j][k] + f[(((i - 1)) & 1)][j][p])) % 998244353);
                if (j)
                {
                  f[(i & 1)][j][k] = (((f[(i & 1)][j][k] + f[(((i - 1)) & 1)][(j - 1)][q])) % 998244353);
                }
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
      var i = 1;
      while ((i <= n2))
      {
        memset(g[(i & 1)], 0, cpp_sizeof(g[(i & 1)]));
        {
          var j = 0;
          while ((j <= i))
          {
            {
              var k = 0;
              while ((k < 11))
              {
                var p = ((((k - b[i]) + 11)) % 11);
                var q = (((k + b[i])) % 11);
                g[(i & 1)][j][k] = (((g[(i & 1)][j][k] + g[(((i - 1)) & 1)][j][p])) % 998244353);
                if (j)
                {
                  g[(i & 1)][j][k] = (((g[(i & 1)][j][k] + g[(((i - 1)) & 1)][(j - 1)][q])) % 998244353);
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ans = 0;
    {
      var i = 0;
      while ((i <= n2))
      {
        {
          var j = 0;
          while ((j < 11))
          {
            ans = (((ans + ((((((((((g[(n2 & 1)][i][j] * f[(n1 & 1)][(n1 / 2)][(((11 - j)) % 11)]) % 998244353) * pw[(n1 / 2)]) % 998244353) * pw[(n1 - (n1 / 2))]) % 998244353) * calc(i, (n1 - (n1 / 2)))) % 998244353) * calc((n2 - i), ((n1 / 2) + 1))) % 998244353))) % 998244353);
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%lld\n", ans);
  }
  return 0;
}

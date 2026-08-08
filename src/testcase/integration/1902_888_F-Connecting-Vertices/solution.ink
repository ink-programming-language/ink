// Translated from solution.cpp.

var mod = (1e9 + 7);

var n: dynamic;

var a = cpp_array(505, 505);

var f = cpp_array(505, 505);

var g = cpp_array(505, 505);

func main()
{
  scanf("%lld", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          scanf("%lld", (&a[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      f[i][i] = 1;
      i += 1;
    }
  }
  {
    var len = 2;
    while ((len <= n))
    {
      {
        var l = 1;
        while ((((l + len) - 1) <= n))
        {
          var r = ((l + len) - 1);
          if (a[l][r])
          {
            {
              var k = l;
              while ((k < r))
              {
                f[l][r] = (((f[l][r] + ((((f[l][k] + g[l][k])) * ((f[(k + 1)][r] + g[(k + 1)][r]))) % mod))) % mod);
                k += 1;
              }
            }
          }
          {
            var k = (l + 1);
            while ((k < r))
            {
              if (a[l][k])
              {
                g[l][r] = (((g[l][r] + ((f[l][k] * ((g[k][r] + f[k][r]))) % mod))) % mod);
              }
              k += 1;
            }
          }
          l += 1;
        }
      }
      len += 1;
    }
  }
  printf("%lld\n", (((f[1][n] + g[1][n])) % mod));
  return 0;
}

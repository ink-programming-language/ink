// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(505, 505);

var f: dynamic = cpp_array(505, 505);

var g: dynamic = cpp_array(505, 505);

func main() -> dynamic
{
  scanf("%lld", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      f[i][i] = 1;
      i += 1;
    }
  }
  {
    var len: dynamic = 2;
    while ((len <= n))
    {
      {
        var l: dynamic = 1;
        while ((((l + len) - 1) <= n))
        {
          var r: dynamic = ((l + len) - 1);
          if (a[l][r])
          {
            {
              var k: dynamic = l;
              while ((k < r))
              {
                f[l][r] = (((f[l][r] + ((((f[l][k] + g[l][k])) * ((f[(k + 1)][r] + g[(k + 1)][r]))) % mod))) % mod);
                k += 1;
              }
            }
          }
          {
            var k: dynamic = (l + 1);
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

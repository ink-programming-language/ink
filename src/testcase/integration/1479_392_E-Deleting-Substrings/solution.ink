// Translated from solution.cpp.

var N = 405;

var inf = 1000000007;

var g = cpp_array(N);

var f = cpp_array(2, N, N);

var n: dynamic;

var a = cpp_array(N);

var v = cpp_array(N);

func dp()
{
  {
    var i = 1;
    while ((i <= n))
    {
      f[i][i][0] = 0;
      f[i][i][1] = v[1];
      i += 1;
    }
  }
  {
    var L = 2;
    while ((L <= n))
    {
      {
        var l = 1;
        while ((l <= ((n - L) + 1)))
        {
          var r = ((l + L) - 1);
          f[l][r][0] = (-inf);
          if ((a[l] != a[r]))
          {
            var st = (if ((a[l] < a[r])) 1 else -1);
            {
              var k = (l + 1);
              while ((k <= r))
              {
                if ((a[k] == (a[l] + st)))
                {
                  f[l][r][0] = max(f[l][r][0], (f[(l + 1)][(k - 1)][1] + f[k][r][0]));
                }
                k += 1;
              }
            }
          }
          f[l][r][1] = (-inf);
          {
            var k = l;
            while ((k <= (r - 1)))
            {
              f[l][r][1] = max(f[l][r][1], (f[l][k][1] + f[(k + 1)][r][1]));
              k += 1;
            }
          }
          {
            var k = l;
            while ((k <= r))
            {
              if ((((a[l] <= a[k]) && (a[r] <= a[k])) && (((((a[k] - a[l]) + a[k]) - a[r]) + 1) <= n)))
              {
                f[l][r][1] = max(f[l][r][1], ((f[l][k][0] + f[k][r][0]) + v[((((a[k] - a[l]) + a[k]) - a[r]) + 1)]));
              }
              k += 1;
            }
          }
          l += 1;
        }
      }
      L += 1;
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&v[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  dp();
  g[0] = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      g[i] = g[(i - 1)];
      {
        var j = 0;
        while ((j <= (i - 1)))
        {
          g[i] = max(g[i], (g[j] + f[(j + 1)][i][1]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", g[n]);
  return 0;
}

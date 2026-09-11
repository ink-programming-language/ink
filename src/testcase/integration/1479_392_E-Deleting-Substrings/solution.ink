// Translated from solution.cpp.

var N: dynamic = 405;

var inf: dynamic = 1000000007;

var g: dynamic = cpp_array(N);

var f: dynamic = cpp_array(2, N, N);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var v: dynamic = cpp_array(N);

func dp() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      f[i][i][0] = 0;
      f[i][i][1] = v[1];
      i += 1;
    }
  }
  {
    var L: dynamic = 2;
    while ((L <= n))
    {
      {
        var l: dynamic = 1;
        while ((l <= ((n - L) + 1)))
        {
          var r: dynamic = ((l + L) - 1);
          f[l][r][0] = (-inf);
          if ((a[l] != a[r]))
          {
            var st: dynamic = ( ((a[l] < a[r])) ? 1 : -1);
            {
              var k: dynamic = (l + 1);
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
            var k: dynamic = l;
            while ((k <= (r - 1)))
            {
              f[l][r][1] = max(f[l][r][1], (f[l][k][1] + f[(k + 1)][r][1]));
              k += 1;
            }
          }
          {
            var k: dynamic = l;
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&v[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  dp();
  g[0] = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      g[i] = g[(i - 1)];
      {
        var j: dynamic = 0;
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

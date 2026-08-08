// Translated from solution.cpp.

var INF = (DBL_MAX / 1000);

var EPS = 1.0e-10;

var L: dynamic;

var a: dynamic;

func calculateError(x: dynamic, y: dynamic)
{
  var vMax = 0.0;
  var vMin = 1.0;
  {
    var i = x;
    while ((i <= y))
    {
      vMax = max(vMax, a[i]);
      vMin = min(vMin, a[i]);
      i += 1;
    }
  }
  var ret = 0.0;
  {
    var i = x;
    while ((i <= y))
    {
      var j = cpp_cast(((((((a[i] - vMin)) * ((L - 1))) / ((vMax - vMin))) + EPS)));
      var q1 = (vMin + ((j * ((vMax - vMin))) / ((L - 1))));
      var q2 = (vMin + ((((j + 1)) * ((vMax - vMin))) / ((L - 1))));
      var d = min(abs((q1 - a[i])), abs((q2 - a[i])));
      d = (d * d);
      ret += d;
      i += 1;
    }
  }
  return ret;
}

func main()
{
  {
    while (true)
    {
      var n: dynamic;
      var m: dynamic;
      read(n, m, L);
      if ((n == 0))
      {
        return 0;
      }
      L = (1 << L);
      a.resize(n);
      {
        var i = 0;
        while ((i < n))
        {
          read(a[i]);
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = (i + 1);
            while ((j < n))
            {
              error[i][j] = calculateError(i, j);
              j += 1;
            }
          }
          i += 1;
        }
      }
      var dp = cpp_construct((n + 1), vector((m + 1), INF));
      dp[0][0] = 0.0;
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < m))
            {
              {
                var k = (i + 1);
                while ((k < n))
                {
                  dp[(k + 1)][(j + 1)] = min(dp[(k + 1)][(j + 1)], (dp[i][j] + error[i][k]));
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      printf("%.10f\n", dp[n][m]);
    }
  }
}

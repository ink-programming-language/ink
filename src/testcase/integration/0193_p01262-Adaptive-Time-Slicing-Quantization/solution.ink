// Translated from solution.cpp.

var INF: dynamic = (DBL_MAX / 1000);

var EPS: dynamic = 1.0e-10;

var L: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

func calculateError(x: dynamic, y: dynamic) -> dynamic
{
  var vMax: dynamic = 0.0;
  var vMin: dynamic = 1.0;
  {
    var i: dynamic = x;
    while ((i <= y))
    {
      vMax = max(vMax, a[i]);
      vMin = min(vMin, a[i]);
      i += 1;
    }
  }
  var ret: dynamic = 0.0;
  {
    var i: dynamic = x;
    while ((i <= y))
    {
      var j: dynamic = cpp_cast(((((((a[i] - vMin)) * ((L - 1))) / ((vMax - vMin))) + EPS)));
      var q1: dynamic = (vMin + ((j * ((vMax - vMin))) / ((L - 1))));
      var q2: dynamic = (vMin + ((((j + 1)) * ((vMax - vMin))) / ((L - 1))));
      var d: dynamic = min(abs((q1 - a[i])), abs((q2 - a[i])));
      d = (d * d);
      ret += d;
      i += 1;
    }
  }
  return ret;
}

func main() -> dynamic
{
  {
    while (true)
    {
      var n: dynamic = cpp_uninitialized();
      var m: dynamic = cpp_uninitialized();
      read(n, m, L);
      if ((n == 0))
      {
        return 0;
      }
      L = (1 << L);
      a.resize(n);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          read(a[i]);
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = (i + 1);
            while ((j < n))
            {
              error[i][j] = calculateError(i, j);
              j += 1;
            }
          }
          i += 1;
        }
      }
      var dp: dynamic = cpp_construct((n + 1), vector((m + 1), INF));
      dp[0][0] = 0.0;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              {
                var k: dynamic = (i + 1);
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

// Translated from solution.cpp.

var N: dynamic;

var K: dynamic;

var dp = cpp_array(1001, 1001, 2);

var dm = cpp_array(1001, 1001, 2);

var a = cpp_array(1000);

func main()
{
  scanf("%d %d", (&N), (&K));
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  sort(a, (a + N));
  fill(cpp_cast(dp[0]), cpp_cast(dp[1]), 1e9);
  var ans = 1e9;
  {
    var T = 1;
    while ((T <= K))
    {
      var t = (T % 2);
      var u = (1 - t);
      {
        var i = 0;
        while ((i < N))
        {
          dp[0][i][(i + 1)] = 0;
          dp[1][i][(i + 1)] = 0;
          i += 1;
        }
      }
      {
        var w = 2;
        while ((w <= N))
        {
          {
            var i = 0;
            while (((i + w) <= N))
            {
              var j = (i + w);
              var si = (i + 1);
              var ti = (j - 1);
              if ((w > 2))
              {
                si = dm[t][i][(j - 1)];
                ti = dm[t][(i + 1)][j];
              }
              dp[t][i][j] = 1e9;
              {
                var k = si;
                while ((k <= ti))
                {
                  if ((dp[t][i][j] > (((dp[u][i][k] + dp[u][k][j]) + a[(j - 1)]) - a[(k - 1)])))
                  {
                    dp[t][i][j] = (((dp[u][i][k] + dp[u][k][j]) + a[(j - 1)]) - a[(k - 1)]);
                    dm[t][i][j] = k;
                  }
                  k += 1;
                }
              }
              i += 1;
            }
          }
          w += 1;
        }
      }
      T += 1;
    }
  }
  printf("%d\n", dp[(K % 2)][0][N]);
  return 0;
}

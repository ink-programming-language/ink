// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var dp: dynamic = cpp_array(200, 20, 30);

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  while (cpp_comma(scanf("%d%d%d", (&N), (&K), (&S)), N))
  {
    memset(dp, 0, cpp_sizeof((dp)));
    {
      var i: dynamic = 0;
      while ((i <= N))
      {
        dp[i][0][0] = 1;
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= N))
      {
        {
          var j: dynamic = 1;
          while ((j <= K))
          {
            {
              var k: dynamic = 0;
              while ((k <= S))
              {
                if ((k < i))
                {
                  dp[i][j][k] = dp[(i - 1)][j][k];
                } else
                {
                  dp[i][j][k] = (dp[(i - 1)][j][k] + dp[(i - 1)][(j - 1)][(k - i)]);
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
    printf("%d\n", dp[N][K][S]);
  }
}

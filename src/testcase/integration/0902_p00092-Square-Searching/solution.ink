// Translated from solution.cpp.

var dp = cpp_array(3000, 3000);

var n: dynamic;

var x = cpp_array(3000, 3000);

var maxn: dynamic;

var a: dynamic;

func main()
{
  while (true)
  {
    memset(dp, 0, cpp_sizeof((dp)));
    read(n);
    if ((!n))
    {
      break;
    }
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
          while ((j <= n))
          {
            read(a);
            if ((a == cpp_char("*")))
            {
              x[i][j] = 1;
            } else
            {
              x[i][j] = 0;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    maxn = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
          while ((j <= n))
          {
            if ((x[i][j] == 1))
            {
              dp[i][j] = 0;
            } else
            {
              dp[i][j] = (min(dp[(i - 1)][(j - 1)], min(dp[(i - 1)][j], dp[i][(j - 1)])) + 1);
            }
            maxn = max(maxn, dp[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(maxn, "\n");
  }
}

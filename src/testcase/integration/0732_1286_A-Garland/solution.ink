// Translated from solution.cpp.

var maxN = 101;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var a = cpp_construct((n + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var dp = cpp_array(2, maxN, maxN);
  memset(dp, 0x3f, cpp_sizeof((dp)));
  dp[0][0][0] = 0;
  dp[0][0][1] = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= n))
        {
          if (((a[i] % 2) || (a[i] == 0)))
          {
            dp[i][j][1] = min((dp[(i - 1)][j][0] + 1), dp[(i - 1)][j][1]);
          }
          if (((a[i] % 2) == 0))
          {
            dp[i][j][0] = min(dp[(i - 1)][(j - 1)][0], (dp[(i - 1)][(j - 1)][1] + 1));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(min(dp[n][(n / 2)][0], dp[n][(n / 2)][1]));
  return 0;
}

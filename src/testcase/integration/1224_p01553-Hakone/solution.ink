// Translated from solution.cpp.

var popcount = cpp_expression("#include <cstdio>");

var MOD = (1e9 + 7);

func main()
{
  var n: dynamic;
  read(n);
  var s: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var c: dynamic;
      read(c);
      if ((c != cpp_char("-")))
      {
        s += c;
      }
      i += 1;
    }
  }
  n = s.size();
  var dp = [];
  dp[0][0] = 1;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j <= i))
        {
          if ((s[i] == cpp_char("U")))
          {
            (cpp_assign(dp[(i + 1)][j], "+=", (j * dp[i][j]))) %= MOD;
            (cpp_assign(dp[(i + 1)][(j + 1)], "+=", dp[i][j])) %= MOD;
          } else
          {
            (cpp_assign(dp[(i + 1)][j], "+=", (j * dp[i][j]))) %= MOD;
            if (j)
            {
              (cpp_assign(dp[(i + 1)][(j - 1)], "+=", ((j * j) * dp[i][j]))) %= MOD;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[n][0], "\n");
  return 0;
}

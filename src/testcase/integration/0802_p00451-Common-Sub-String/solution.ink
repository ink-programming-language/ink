// Translated from solution.cpp.

var int_cpp = dynamic;

var mod = cpp_expression("#include<i");

var s: dynamic;

var t: dynamic;

var dp = cpp_array(4005, 4005);

var ans: dynamic;

func main()
{
  while (((cin >> s) >> t))
  {
    {
      var i = 0;
      while ((i <= 4000))
      {
        {
          var j = 0;
          while ((j <= 4000))
          {
            dp[i][j] = 0;
            j += 1;
          }
        }
        i += 1;
      }
    }
    ans = 0;
    {
      var i = 1;
      while ((i <= s.size()))
      {
        {
          var j = 1;
          while ((j <= t.size()))
          {
            if ((s[(i - 1)] == t[(j - 1)]))
            {
              dp[i][j] = (dp[(i - 1)][(j - 1)] + 1);
              ans = max(dp[i][j], ans);
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
}

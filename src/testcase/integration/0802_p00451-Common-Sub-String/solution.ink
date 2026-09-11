// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var mod: dynamic = cpp_expression("#include<i");

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(4005, 4005);

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while (((cin >> s) >> t))
  {
    {
      var i: dynamic = 0;
      while ((i <= 4000))
      {
        {
          var j: dynamic = 0;
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
      var i: dynamic = 1;
      while ((i <= s.size()))
      {
        {
          var j: dynamic = 1;
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

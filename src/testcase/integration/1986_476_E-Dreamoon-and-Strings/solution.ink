// Translated from solution.cpp.

var s: dynamic;

var p: dynamic;

var a = cpp_array(2005);

var A = cpp_array(2005);

var ans = cpp_array(2005);

var dp = cpp_array(2005, 2005);

func main()
{
  read(s, p);
  memset(a, -1, cpp_sizeof((a)));
  a[0] = 0;
  {
    var i = 0;
    while ((i < s.size()))
    {
      {
        var j = (p.size() - 1);
        while ((j >= 0))
        {
          if ((s[i] == p[j]))
          {
            a[(j + 1)] = a[j];
          }
          j -= 1;
        }
      }
      a[0] = (i + 1);
      A[(i + 1)] = a[p.size()];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= s.size()))
    {
      {
        var j = 0;
        while ((j <= i))
        {
          dp[i][j] = dp[(i - 1)][j];
          if ((((A[i] >= 0) && ((j - (((i - A[i]) - p.size()))) >= 0)) && ((j - (((i - A[i]) - p.size()))) <= A[i])))
          {
            dp[i][j] = max((dp[A[i]][(j - (((i - A[i]) - p.size())))] + 1), dp[i][j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= s.size()))
    {
      ans[i] = dp[s.size()][i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= s.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}

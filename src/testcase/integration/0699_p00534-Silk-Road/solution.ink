// Translated from solution.cpp.

func loop(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func rep(i: dynamic, a: dynamic)
{
  return cpp_expression("#include<io");
}

var INF = cpp_expression("#inc");

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var dp = cpp_array((m + 1), (n + 1));
  var d = cpp_array((n + 1));
  var c = cpp_array((m + 1));
  rep(i, (n + 1));
  {
    cpp_statement("rep(j,m+1)");
    {
      if ((i == 0))
      {
        dp[i][j] = 0;
      } else
      {
        dp[i][j] = INF;
      }
    }
  }
  loop(i, 1, (n + 1));
  read(d[i]);
  loop(i, 1, (m + 1));
  read(c[i]);
  loop(i, 1, (n + 1));
  {
    loop(j, 1, (m + 1));
    {
      dp[i][j] = min(dp[i][(j - 1)], (dp[(i - 1)][(j - 1)] + (d[i] * c[j])));
    }
  }
  write(dp[n][m], "\n");
  return 0;
}

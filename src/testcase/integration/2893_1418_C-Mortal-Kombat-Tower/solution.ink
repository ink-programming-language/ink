// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  var n: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var arr = cpp_array(n);
    var dp = cpp_array(2, 2, n);
    {
      var i = 0;
      while ((i < n))
      {
        read(arr[i]);
        i += 1;
      }
    }
    dp[0][0][0] = arr[0];
    dp[0][0][1] = (2 * n);
    dp[0][1][0] = (2 * n);
    dp[0][1][1] = (2 * n);
    {
      var i = 1;
      while ((i < n))
      {
        dp[i][0][0] = min(dp[(i - 1)][1][0], dp[(i - 1)][1][1]);
        dp[i][0][1] = (arr[i] + dp[(i - 1)][0][0]);
        dp[i][1][0] = min(dp[(i - 1)][0][0], dp[(i - 1)][0][1]);
        dp[i][1][1] = dp[(i - 1)][1][0];
        if ((i != 1))
        {
          dp[i][0][0] += arr[i];
        }
        i += 1;
      }
    }
    write(min(min(dp[(n - 1)][0][0], dp[(n - 1)][0][1]), min(dp[(n - 1)][1][0], dp[(n - 1)][1][1])), "\n");
  }
  return 0;
}

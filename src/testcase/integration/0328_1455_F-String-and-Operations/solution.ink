// Translated from solution.cpp.

func debug()
{
  return cpp_expression("#inclu");
}

func main()
{
  ios.sync_with_stdio(0);
  cout.tie(0);
  cin.tie(0);
  var tc: dynamic;
  read(tc);
  while (cpp_update(tc, "--"))
  {
    var n: dynamic;
    var k: dynamic;
    var s: dynamic;
    read(n, k, s);
    var dp = cpp_construct((n + 1));
    var c = __cpp_lambda_1;
    {
      var i = 0;
      while ((i < n))
      {
        if ((dp[(i + 1)].size() == 0))
        {
          dp[(i + 1)] = (dp[i] + s[i]);
        }
        {
          var j = -1;
          while ((j <= 1))
          {
            dp[(i + 1)] = min(dp[(i + 1)], (dp[i] + c(s[i], j)));
            j += 1;
          }
        }
        if ((i > 0))
        {
          var tmp = (dp[i] + s[i]);
          swap(tmp[i], tmp[(i - 1)]);
          dp[(i + 1)] = min(dp[(i + 1)], tmp);
        }
        if (((i + 2) > n))
        {
          i += 1;
          continue;
        }
        if ((dp[(i + 2)].size() == 0))
        {
          dp[(i + 2)] = ((dp[i] + s[i]) + s[(i + 1)]);
        }
        {
          var j = -1;
          while ((j <= 1))
          {
            dp[(i + 2)] = min(dp[(i + 2)], ((dp[i] + c(s[(i + 1)], j)) + s[i]));
            j += 1;
          }
        }
        if (((i > 0) && ((i + 1) < n)))
        {
          var tmp = (dp[i] + s[(i + 1)]);
          swap(tmp[i], tmp[(i - 1)]);
          dp[(i + 2)] = min(dp[(i + 2)], (tmp + s[i]));
        }
        i += 1;
      }
    }
    debug(dp);
    write(dp[n], cpp_char("\n"));
  }
}

func __cpp_lambda_1(c: dynamic, x: dynamic)
{
  return cpp_assign(string_cpp(), "=", ((cpp_char("a") + (((((c - cpp_char("a")) + x) + k)) % k))));
}

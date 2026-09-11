// Translated from solution.cpp.

func debug() -> dynamic
{
  return cpp_expression("#inclu");
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cout.tie(0);
  cin.tie(0);
  var tc: dynamic = cpp_uninitialized();
  read(tc);
  while (cpp_update(tc, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var s: dynamic = cpp_uninitialized();
    read(n, k, s);
    var dp: dynamic = cpp_construct((n + 1));
    var c: dynamic = __cpp_lambda_1;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((dp[(i + 1)].size() == 0))
        {
          dp[(i + 1)] = (dp[i] + s[i]);
        }
        {
          var j: dynamic = -1;
          while ((j <= 1))
          {
            dp[(i + 1)] = min(dp[(i + 1)], (dp[i] + c(s[i], j)));
            j += 1;
          }
        }
        if ((i > 0))
        {
          var tmp: dynamic = (dp[i] + s[i]);
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
          var j: dynamic = -1;
          while ((j <= 1))
          {
            dp[(i + 2)] = min(dp[(i + 2)], ((dp[i] + c(s[(i + 1)], j)) + s[i]));
            j += 1;
          }
        }
        if (((i > 0) && ((i + 1) < n)))
        {
          var tmp: dynamic = (dp[i] + s[(i + 1)]);
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

func __cpp_lambda_1(c: dynamic, x: dynamic) -> dynamic
{
  return cpp_assign(string_cpp(), "=", ((cpp_char("a") + (((((c - cpp_char("a")) + x) + k)) % k))));
}

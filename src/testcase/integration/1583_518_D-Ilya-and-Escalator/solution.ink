// Translated from solution.cpp.

var modulo: dynamic = 1000000007;

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  read(n, p, t);
  var dp: dynamic = cpp_construct((t + 1), vector((n + 1), 0.0));
  dp[0][0] = 1.0;
  {
    var i: dynamic = 0;
    while ((i <= (t - 1)))
    {
      {
        var j: dynamic = 0;
        while ((j <= (n - 1)))
        {
          dp[(i + 1)][(j + 1)] += (p * dp[i][j]);
          dp[(i + 1)][j] += (((1 - p)) * dp[i][j]);
          j += 1;
        }
      }
      dp[(i + 1)][n] += dp[i][n];
      i += 1;
    }
  }
  var ans: dynamic = 0.0;
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      ans += (dp[t][i] * i);
      i += 1;
    }
  }
  cout.precision(9);
  write(fixed, ans, "\n");
  return 0;
}

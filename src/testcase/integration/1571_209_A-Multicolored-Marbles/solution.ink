// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  read(n);
  var dp: dynamic = cpp_array(2, (n + 1));
  var m: dynamic = 1000000007;
  dp[0][0] = 0;
  dp[0][1] = 0;
  {
    i = 1;
    while ((i <= n))
    {
      dp[i][(i % 2)] = ((((dp[(i - 1)][(1 - ((i % 2)))] + 1) + dp[(i - 1)][((i % 2))])) % m);
      dp[i][(1 - ((i % 2)))] = dp[(i - 1)][(1 - ((i % 2)))];
      i += 1;
    }
  }
  write((((dp[n][0] + dp[n][1])) % m));
}

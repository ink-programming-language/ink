// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var L: dynamic = cpp_array(222222);
  var R: dynamic = cpp_array(222222);
  read(N, K);
  var i: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i <= (K - 1)))
    {
      read(L[i], R[i]);
      i += 1;
    }
  }
  var dp: dynamic = [0];
  dp[1] = 1;
  var sum: dynamic = [0];
  sum[1] = 1;
  {
    i = 2;
    while ((i <= N))
    {
      {
        k = 0;
        while ((k <= (K - 1)))
        {
          dp[i] = ((((dp[i] + sum[(i - L[k])]) - sum[((i - R[k]) - 1)])) % 998244353);
          sum[i] = (((sum[(i - 1)] + dp[i])) % 998244353);
          k += 1;
        }
      }
      i += 1;
    }
  }
  if ((dp[N] < 0))
  {
    dp[N] += 998244353;
  }
  write(dp[N], "\n");
}

// Translated from solution.cpp.

var MOD = 1000000007;

func mod_pow(A: dynamic, k: dynamic)
{
  var res = 1;
  {
    while ((k > 0))
    {
      if ((k & 1))
      {
        (cpp_assign(res, "*=", A)) %= MOD;
      }
      (cpp_assign(A, "*=", A)) %= MOD;
      k >>= 1;
    }
  }
  return res;
}

var dp = cpp_array(100010);

func main()
{
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  dp[0] = 1;
  {
    var i = 0;
    while ((i < N))
    {
      dp[(i + 1)] = (dp[i] - (if (((i - M) < 0)) 0 else dp[(i - M)]));
      (cpp_assign(dp[(i + 1)], "+=", MOD)) %= MOD;
      (cpp_assign(dp[(i + 1)], "+=", dp[i])) %= MOD;
      i += 1;
    }
  }
  var ans = mod_pow(2, N);
  var sub = ((((dp[N] - dp[(N - M)]) + MOD)) % MOD);
  (cpp_assign(ans, "+=", (MOD - sub))) %= MOD;
  write(ans, "\n");
  return 0;
}

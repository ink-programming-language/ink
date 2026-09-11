// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

func mod_pow(A: dynamic, k: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

var dp: dynamic = cpp_array(100010);

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  read(N, M);
  dp[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      dp[(i + 1)] = (dp[i] - ( (((i - M) < 0)) ? 0 : dp[(i - M)]));
      (cpp_assign(dp[(i + 1)], "+=", MOD)) %= MOD;
      (cpp_assign(dp[(i + 1)], "+=", dp[i])) %= MOD;
      i += 1;
    }
  }
  var ans: dynamic = mod_pow(2, N);
  var sub: dynamic = ((((dp[N] - dp[(N - M)]) + MOD)) % MOD);
  (cpp_assign(ans, "+=", (MOD - sub))) %= MOD;
  write(ans, "\n");
  return 0;
}

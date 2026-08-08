// Translated from solution.cpp.

var MOD = (((1000 * 1000) * 1000) + 7);

func main()
{
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  for (var x in S)
  {
    read(x);
  }
  for (var x in T)
  {
    read(x);
  }
  var dp = cpp_construct((N + 1), vector((M + 1), 0));
  {
    var i = 0;
    while ((i <= N))
    {
      dp[i][0] = 1;
      i += 1;
    }
  }
  {
    var j = 0;
    while ((j <= M))
    {
      dp[0][j] = 1;
      j += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j < M))
        {
          dp[(i + 1)][(j + 1)] = (((dp[i][(j + 1)] + dp[(i + 1)][j])) % MOD);
          if ((S[i] != T[j]))
          {
            dp[(i + 1)][(j + 1)] = ((((dp[(i + 1)][(j + 1)] + MOD) - dp[i][j])) % MOD);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[N][M], "\n");
}

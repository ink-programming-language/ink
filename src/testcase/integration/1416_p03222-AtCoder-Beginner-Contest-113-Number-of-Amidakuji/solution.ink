// Translated from solution.cpp.

var MOD = (1e9 + 7);

func main()
{
  var H: dynamic;
  var W: dynamic;
  var K: dynamic;
  scanf("%zu %zu %zu", (&H), (&W), (&K));
  var dp = cpp_construct((H + 1), vector(W));
  dp[0][0] = 1;
  {
    var i = 0;
    while ((i < H))
    {
      {
        var j = 0;
        while ((j < W))
        {
          {
            var k = 0;
            while ((k < ((1 << ((W - 1))))))
            {
              if ((k & ((k >> 1))))
              {
                k += 1;
                continue;
              }
              if ((((j + 1) < W) && ((k >> j) & 1)))
              {
                (cpp_assign(dp[(i + 1)][(j + 1)], "+=", dp[i][j])) %= MOD;
              } else if (((j > 0) && ((k >> ((j - 1))) & 1)))
              {
                (cpp_assign(dp[(i + 1)][(j - 1)], "+=", dp[i][j])) %= MOD;
              } else
              {
                (cpp_assign(dp[(i + 1)][j], "+=", dp[i][j])) %= MOD;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%jd\n", dp[H][(K - 1)]);
}

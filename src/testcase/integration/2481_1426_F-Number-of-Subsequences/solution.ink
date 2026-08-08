// Translated from solution.cpp.

var N = (2e5 + 10);

var dp = cpp_array(5, N);

var sum = cpp_array(5, N);

var three = [0];

var MOD = (1e9 + 7);

var s = cpp_array(N);

func main()
{
  three[0] = 1;
  {
    var i = 1;
    while ((i < N))
    {
      three[i] = ((three[(i - 1)] * 3) % MOD);
      i += 1;
    }
  }
  var n: dynamic;
  var num = 0;
  scanf("%d %s", (&n), (s + 1));
  sum[0][0] = cpp_assign(dp[0][0], "=", 1);
  {
    var i = 1;
    while ((i <= n))
    {
      if ((s[i] == cpp_char("?")))
      {
        sum[i][0] = 1;
        sum[i][1] = ((((sum[(i - 1)][1] * 3) + three[num])) % MOD);
        sum[i][2] = ((((sum[(i - 1)][2] * 3) + sum[(i - 1)][1])) % MOD);
        sum[i][3] = ((((sum[(i - 1)][3] * 3) + sum[(i - 1)][2])) % MOD);
        num += 1;
      } else
      {
        if ((s[i] == cpp_char("a")))
        {
          dp[i][((s[i] - cpp_char("a")) + 1)] = ((sum[(i - 1)][(s[i] - cpp_char("a"))] * three[num]) % MOD);
        } else
        {
          dp[i][((s[i] - cpp_char("a")) + 1)] = (sum[(i - 1)][(s[i] - cpp_char("a"))] % MOD);
        }
        {
          var j = 0;
          while ((j <= 3))
          {
            sum[i][j] = (((sum[(i - 1)][j] + dp[i][j])) % MOD);
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  printf("%lld\n", sum[n][3]);
}

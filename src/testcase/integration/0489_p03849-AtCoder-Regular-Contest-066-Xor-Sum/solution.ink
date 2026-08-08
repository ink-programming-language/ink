// Translated from solution.cpp.

var MOD = (1e9 + 7);

func add(a: dynamic, b: dynamic)
{
  a = (((a + b)) % MOD);
}

func nth_bit(num: dynamic, n: dynamic)
{
  return (((num >> n)) & 1);
}

func main()
{
  var N: dynamic;
  read(N);
  var dp = [0];
  dp[60][0] = 1;
  {
    var d = 59;
    while ((d >= 0))
    {
      {
        var s = 0;
        while ((s <= 2))
        {
          {
            var k = 0;
            while ((k <= 2))
            {
              var s2 = min(2, (((2 * s) + nth_bit(N, d)) - k));
              if ((s2 >= 0))
              {
                add(dp[d][s2], dp[(d + 1)][s]);
              }
              k += 1;
            }
          }
          s += 1;
        }
      }
      d -= 1;
    }
  }
  var ans = ((((dp[0][0] + dp[0][1]) + dp[0][2])) % MOD);
  write(ans, "\n");
  return 0;
}

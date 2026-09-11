// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

func add(a: dynamic, b: dynamic) -> dynamic
{
  a = (((a + b)) % MOD);
}

func nth_bit(num: dynamic, n: dynamic) -> dynamic
{
  return (((num >> n)) & 1);
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var dp: dynamic = [0];
  dp[60][0] = 1;
  {
    var d: dynamic = 59;
    while ((d >= 0))
    {
      {
        var s: dynamic = 0;
        while ((s <= 2))
        {
          {
            var k: dynamic = 0;
            while ((k <= 2))
            {
              var s2: dynamic = min(2, (((2 * s) + nth_bit(N, d)) - k));
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
  var ans: dynamic = ((((dp[0][0] + dp[0][1]) + dp[0][2])) % MOD);
  write(ans, "\n");
  return 0;
}

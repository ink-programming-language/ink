// Translated from solution.cpp.

var kMod: dynamic = (1e9 + 7);

var dp: dynamic = cpp_array(301, 301, 301);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  dp[0][(n - 1)][1] = 1;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = n;
        while ((j >= 0))
        {
          {
            var k: dynamic = 1;
            while ((k <= n))
            {
              dp[i][j][k] %= kMod;
              dp[(i + 1)][j][k] += (dp[i][j][k] * (((n - j) - k)));
              if ((j > 0))
              {
                dp[(i + 1)][(j - 1)][k] += (dp[i][j][k] * j);
              }
              dp[(i + 1)][j][(n - j)] += (dp[i][j][k] * k);
              k += 1;
            }
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  write((dp[m][0][n] % kMod), "\n");
}

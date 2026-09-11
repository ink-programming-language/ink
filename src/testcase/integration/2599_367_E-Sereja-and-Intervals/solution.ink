// Translated from solution.cpp.

var NMAX: dynamic = 321;

var MOD: dynamic = (1e9 + 7);

var dp: dynamic = cpp_array(NMAX, NMAX, 2);

var tdp: dynamic = cpp_array(NMAX, NMAX, 2);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n, m, x);
  if ((n > m))
  {
    write("0\n");
    return 0;
  }
  dp[0][0][0] = cpp_assign(tdp[0][0][0], "=", 1);
  {
    var i: dynamic = 1;
    var u: dynamic = 1;
    while ((i <= m))
    {
      {
        var j: dynamic = 0;
        while ((j <= n))
        {
          {
            var k: dynamic = 0;
            while ((k <= n))
            {
              dp[u][j][k] = dp[(u ^ 1)][j][k];
              tdp[u][j][k] = tdp[(u ^ 1)][j][k];
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j <= n))
        {
          {
            var k: dynamic = 0;
            while ((k <= n))
            {
              if ((j > 0))
              {
                dp[u][j][k] += dp[(u ^ 1)][(j - 1)][(k + 1)];
                tdp[u][j][k] += tdp[(u ^ 1)][(j - 1)][(k + 1)];
              }
              if ((k > 0))
              {
                if ((i != x))
                {
                  dp[u][j][k] += dp[(u ^ 1)][j][(k - 1)];
                }
                tdp[u][j][k] += tdp[(u ^ 1)][j][(k - 1)];
              }
              if ((j > 0))
              {
                if ((i != x))
                {
                  dp[u][j][k] += dp[(u ^ 1)][(j - 1)][k];
                }
                tdp[u][j][k] += tdp[(u ^ 1)][(j - 1)][k];
              }
              dp[u][j][k] %= MOD;
              tdp[u][j][k] %= MOD;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
      u ^= 1;
    }
  }
  var ans: dynamic = (((tdp[(m & 1)][n][0] - dp[(m & 1)][n][0])) % MOD);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans = ((cpp_cast(ans) * i) % MOD);
      i += 1;
    }
  }
  if ((ans < 0))
  {
    ans += MOD;
  }
  write(ans, cpp_char("\n"));
}

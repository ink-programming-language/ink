// Translated from solution.cpp.

var N = (2e5 + 5);

var inf = 1e18;

var mod = 998244353;

var n: dynamic;

var dp = cpp_array(2, 202, 2);

var ps = cpp_array(2, 202, 2);

var a = cpp_array(N);

var PS = cpp_array(2, 202, 2);

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  dp[0][0][1] = 1;
  {
    var j = 0;
    while ((j <= 200))
    {
      ps[0][j][1] = 1;
      j += 1;
    }
  }
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      if ((a[i] != -1))
      {
        dp[(i % 2)][a[i]][0] = (ps[(((i - 1)) % 2)][(a[i] - 1)][0] + ps[(((i - 1)) % 2)][(a[i] - 1)][1]);
        dp[(i % 2)][a[i]][0] %= mod;
        dp[(i % 2)][a[i]][1] = (dp[(((i - 1)) % 2)][a[i]][0] + PS[(((i - 1)) % 2)][a[i]][1]);
        dp[(i % 2)][a[i]][1] %= mod;
      } else
      {
        {
          var j = 1;
          while ((j <= 200))
          {
            dp[(i % 2)][j][0] = (ps[(((i - 1)) % 2)][(j - 1)][0] + ps[(((i - 1)) % 2)][(j - 1)][1]);
            dp[(i % 2)][j][0] %= mod;
            dp[(i % 2)][j][1] = (dp[(((i - 1)) % 2)][j][0] + PS[(((i - 1)) % 2)][j][1]);
            dp[(i % 2)][j][1] %= mod;
            j += 1;
          }
        }
      }
      ps[0][0][1] = 0;
      {
        var j = 1;
        while ((j <= 200))
        {
          ps[(i % 2)][j][0] = (ps[(i % 2)][(j - 1)][0] + dp[(i % 2)][j][0]);
          ps[(i % 2)][j][0] %= mod;
          ps[(i % 2)][j][1] = (ps[(i % 2)][(j - 1)][1] + dp[(i % 2)][j][1]);
          ps[(i % 2)][j][1] %= mod;
          j += 1;
        }
      }
      {
        var j = 200;
        while ((j >= 1))
        {
          PS[(i % 2)][j][1] = (PS[(i % 2)][(j + 1)][1] + dp[(i % 2)][j][1]);
          PS[(i % 2)][j][1] %= mod;
          j -= 1;
        }
      }
      {
        var j = 0;
        while ((j <= 200))
        {
          dp[(((i - 1)) % 2)][j][0] = cpp_assign(dp[(((i - 1)) % 2)][j][1], "=", 0);
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ps[(n % 2)][200][1]);
  return 0;
}

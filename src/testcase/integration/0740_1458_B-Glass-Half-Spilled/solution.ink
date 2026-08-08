// Translated from solution.cpp.

var USE_MATH_DEFINES = cpp_expression("#def");

var N = 102;

var a = cpp_array(N);

var b = cpp_array(N);

var dp = cpp_array(N, (N * N), N);

var n: dynamic;

var watersum: dynamic;

var volumesum: dynamic;

func main()
{
  cin.tie(0);
  cin.sync_with_stdio(0);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      watersum += b[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      volumesum += a[i];
      i += 1;
    }
  }
  memset(dp, -1, cpp_sizeof((dp)));
  dp[0][0][0] = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j <= volumesum))
        {
          {
            var k = 0;
            while ((k <= n))
            {
              if ((~dp[i][j][k]))
              {
                dp[(i + 1)][j][k] = max(dp[(i + 1)][j][k], dp[i][j][k]);
                dp[(i + 1)][(j + a[i])][(k + 1)] = max(dp[(i + 1)][(j + a[i])][(k + 1)], (dp[i][j][k] + b[i]));
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
  var res = cpp_construct((n + 1));
  {
    var k = 1;
    while ((k <= n))
    {
      {
        var j = 0;
        while ((j <= volumesum))
        {
          if ((~dp[n][j][k]))
          {
            res[k] = max(res[k], min(ld(j), (dp[n][j][k] + (ld((watersum - dp[n][j][k])) / 2))));
          }
          j += 1;
        }
      }
      k += 1;
    }
  }
  {
    var k = 1;
    while ((k <= n))
    {
      write(fixed, setprecision(10), res[k], " ");
      k += 1;
    }
  }
  write("\n");
}

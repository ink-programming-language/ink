// Translated from solution.cpp.

var USE_MATH_DEFINES: dynamic = cpp_expression("#def");

var N: dynamic = 102;

var a: dynamic = cpp_array(N);

var b: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(N, (N * N), N);

var n: dynamic = cpp_uninitialized();

var watersum: dynamic = cpp_uninitialized();

var volumesum: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  cin.tie(0);
  cin.sync_with_stdio(0);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      watersum += b[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      volumesum += a[i];
      i += 1;
    }
  }
  memset(dp, -1, cpp_sizeof((dp)));
  dp[0][0][0] = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j <= volumesum))
        {
          {
            var k: dynamic = 0;
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
  var res: dynamic = cpp_construct((n + 1));
  {
    var k: dynamic = 1;
    while ((k <= n))
    {
      {
        var j: dynamic = 0;
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
    var k: dynamic = 1;
    while ((k <= n))
    {
      write(fixed, setprecision(10), res[k], " ");
      k += 1;
    }
  }
  write("\n");
}

// Translated from solution.cpp.

var pb = cpp_expression("/* cerber");

func fast_cin()
{
  cpp_macro("ios_base::sync_with_stdio(false); cin.tie(NULL)");
}

var N = (500 + 10);

var a = cpp_array(N, N);

var f = cpp_array(N, N);

var b = cpp_array(N, N);

var dp = cpp_array(N, N);

func main()
{
  fast_cin();
  var n: dynamic;
  read(n);
  var sum = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          if ((i != j))
          {
            read(a[i][j]);
            sum += a[i][j];
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          f[i][j] = (f[i][(j - 1)] + a[j][i]);
          j += 1;
        }
      }
      {
        var j = (i - 1);
        while ((j >= 1))
        {
          b[i][j] = (b[i][(j + 1)] + a[i][j]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i < n))
    {
      {
        var j = i;
        while ((j >= 0))
        {
          {
            var k = j;
            while ((k >= 0))
            {
              var cost = ((dp[j][k] + f[(i + 1)][j]) + b[(i + 1)][(k + 1)]);
              dp[(i + 1)][j] = max(dp[(i + 1)][j], cost);
              dp[j][k] = cost;
              ans = max(ans, cost);
              k -= 1;
            }
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  write((sum - ans), "\n");
}

// Translated from solution.cpp.

var pb: dynamic = cpp_expression("/* cerber");

func fast_cin() -> dynamic
{
  cpp_macro("ios_base::sync_with_stdio(false); cin.tie(NULL)");
}

var N: dynamic = (500 + 10);

var a: dynamic = cpp_array(N, N);

var f: dynamic = cpp_array(N, N);

var b: dynamic = cpp_array(N, N);

var dp: dynamic = cpp_array(N, N);

func main() -> dynamic
{
  fast_cin();
  var n: dynamic = cpp_uninitialized();
  read(n);
  var sum: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          f[i][j] = (f[i][(j - 1)] + a[j][i]);
          j += 1;
        }
      }
      {
        var j: dynamic = (i - 1);
        while ((j >= 1))
        {
          b[i][j] = (b[i][(j + 1)] + a[i][j]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      {
        var j: dynamic = i;
        while ((j >= 0))
        {
          {
            var k: dynamic = j;
            while ((k >= 0))
            {
              var cost: dynamic = ((dp[j][k] + f[(i + 1)][j]) + b[(i + 1)][(k + 1)]);
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

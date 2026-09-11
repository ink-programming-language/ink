// Translated from solution.cpp.

var INF: dynamic = cpp_expression("#incl");

var LINF: dynamic = cpp_expression("#includ");

var MAX_C: dynamic = cpp_expression("#in");

var Min: dynamic = cpp_expression("#incl");

var dp: dynamic = cpp_array(Min, MAX_C);

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  read(N, C);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(a[i], b[i], c[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < MAX_C))
    {
      {
        var j: dynamic = 0;
        while ((j < Min))
        {
          dp[i][j] = (-INF);
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0][0] = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = (MAX_C - 1);
        while ((j >= 0))
        {
          {
            var k: dynamic = (Min - 1);
            while ((k >= 0))
            {
              if ((dp[j][k] == (-INF)))
              {
                k -= 1;
                continue;
              }
              if (((j + c[i]) > C))
              {
                k -= 1;
                continue;
              }
              var x: dynamic = min((k + a[i]), (dp[j][k] + b[i]));
              var y: dynamic = max((k + a[i]), (dp[j][k] + b[i]));
              dp[(j + c[i])][x] = max(dp[(j + c[i])][x], y);
              x = min((k + b[i]), (dp[j][k] + a[i]));
              y = max((k + b[i]), (dp[j][k] + a[i]));
              dp[(j + c[i])][x] = max(dp[(j + c[i])][x], y);
              k -= 1;
            }
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < MAX_C))
    {
      {
        var j: dynamic = (Min - 1);
        while ((j >= 0))
        {
          if ((dp[i][j] == (-INF)))
          {
            j -= 1;
            continue;
          }
          ans = max(ans, cpp_cast(j));
          break;
          j -= 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

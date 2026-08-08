// Translated from solution.cpp.

var INF = cpp_expression("#incl");

var LINF = cpp_expression("#includ");

var MAX_C = cpp_expression("#in");

var Min = cpp_expression("#incl");

var dp = cpp_array(Min, MAX_C);

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  var C: dynamic;
  read(N, C);
  {
    var i = 0;
    while ((i < N))
    {
      read(a[i], b[i], c[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < MAX_C))
    {
      {
        var j = 0;
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
    var i = 0;
    while ((i < N))
    {
      {
        var j = (MAX_C - 1);
        while ((j >= 0))
        {
          {
            var k = (Min - 1);
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
              var x = min((k + a[i]), (dp[j][k] + b[i]));
              var y = max((k + a[i]), (dp[j][k] + b[i]));
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
  var ans = 0;
  {
    var i = 0;
    while ((i < MAX_C))
    {
      {
        var j = (Min - 1);
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

// Translated from solution.cpp.

var MOD = cpp_expression("#include<i");

var MAX_N = cpp_expression("#in");

var MAX_W = cpp_expression("#incl");

var dp = cpp_array(MAX_W, MAX_N, 2);

var N: dynamic;

var W: dynamic;

var x = cpp_array(MAX_N);

func main()
{
  read(N, W);
  {
    var i = 1;
    while ((i <= N))
    {
      read(x[i]);
      i += 1;
    }
  }
  x[(N + 1)] = (W + 1);
  sort((x + 1), ((x + N) + 1));
  dp[1][1][0] = 1;
  {
    var i = 1;
    while ((i <= N))
    {
      {
        var j = 0;
        while ((j <= N))
        {
          {
            var k = 0;
            while ((k <= W))
            {
              dp[0][j][k] = dp[1][j][k];
              dp[1][j][k] = 0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      var w = x[i];
      var y = 0;
      var z = 0;
      {
        var j = 1;
        while ((j <= i))
        {
          y = j;
          z = j;
          if ((y == i))
          {
            y += 1;
          }
          {
            var k = 0;
            while ((k <= W))
            {
              if (((k + w) <= W))
              {
                dp[1][y][(k + w)] += dp[0][j][k];
                dp[1][y][(k + w)] %= MOD;
              }
              dp[1][z][k] += dp[0][j][k];
              dp[1][z][k] %= MOD;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var res = 0;
  var v = 0;
  {
    var i = 1;
    while ((i <= (N + 1)))
    {
      v = max(0, ((W - x[i]) + 1));
      {
        var j = v;
        while ((j <= W))
        {
          res += dp[1][i][j];
          res %= MOD;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}

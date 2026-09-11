// Translated from solution.cpp.

var MOD: dynamic = cpp_expression("#include<i");

var MAX_N: dynamic = cpp_expression("#in");

var MAX_W: dynamic = cpp_expression("#incl");

var dp: dynamic = cpp_array(MAX_W, MAX_N, 2);

var N: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var x: dynamic = cpp_array(MAX_N);

func main() -> dynamic
{
  read(N, W);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= N))
    {
      {
        var j: dynamic = 0;
        while ((j <= N))
        {
          {
            var k: dynamic = 0;
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
      var w: dynamic = x[i];
      var y: dynamic = 0;
      var z: dynamic = 0;
      {
        var j: dynamic = 1;
        while ((j <= i))
        {
          y = j;
          z = j;
          if ((y == i))
          {
            y += 1;
          }
          {
            var k: dynamic = 0;
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
  var res: dynamic = 0;
  var v: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= (N + 1)))
    {
      v = max(0, ((W - x[i]) + 1));
      {
        var j: dynamic = v;
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

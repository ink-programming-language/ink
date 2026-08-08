// Translated from solution.cpp.

var LMT = 202;

var zero = 200;

var rest = cpp_array(LMT);

var dp = cpp_array((LMT << 1), LMT, LMT);

var p = cpp_array(LMT);

func main()
{
  var n: dynamic;
  var l: dynamic;
  var k: dynamic;
  var x: dynamic;
  var ans = 0;
  scanf("%d%d%d", (&n), (&l), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&x));
      p[i] = (x / 100.0);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&rest[i]));
      i += 1;
    }
  }
  var end = (zero + 200);
  dp[0][0][(zero + k)] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j < i))
        {
          {
            var k = 0;
            while ((k <= end))
            {
              var x = min(end, (k + rest[i]));
              dp[i][(j + 1)][x] += (dp[(i - 1)][j][k] * p[i]);
              dp[i][j][k] += (dp[(i - 1)][j][k] * ((1 - p[i])));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = l;
    while ((i <= n))
    {
      {
        var j = zero;
        while ((j <= end))
        {
          ans += dp[n][i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%.12lf\n", ans);
  return 0;
}

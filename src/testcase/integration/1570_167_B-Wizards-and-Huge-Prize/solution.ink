// Translated from solution.cpp.

var LMT: dynamic = 202;

var zero: dynamic = 200;

var rest: dynamic = cpp_array(LMT);

var dp: dynamic = cpp_array((LMT << 1), LMT, LMT);

var p: dynamic = cpp_array(LMT);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  scanf("%d%d%d", (&n), (&l), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&x));
      p[i] = (x / 100.0);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&rest[i]));
      i += 1;
    }
  }
  var end: dynamic = (zero + 200);
  dp[0][0][(zero + k)] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j < i))
        {
          {
            var k: dynamic = 0;
            while ((k <= end))
            {
              var x: dynamic = min(end, (k + rest[i]));
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
    var i: dynamic = l;
    while ((i <= n))
    {
      {
        var j: dynamic = zero;
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

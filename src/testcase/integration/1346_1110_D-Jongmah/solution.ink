// Translated from solution.cpp.

var a = cpp_array(1000005);

var dp = cpp_array(3, 3, 1000005);

var inf = 0x3f3f3f3f;

func main()
{
  var n: dynamic;
  var m: dynamic;
  var d: dynamic;
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&d));
      a[d] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= (m + 1)))
    {
      {
        var j = 0;
        while ((j < 3))
        {
          {
            var k = 0;
            while ((k < 3))
            {
              dp[i][j][k] = (-inf);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[1][0][0] = 0;
  {
    var i = 1;
    while ((i <= m))
    {
      {
        var j = 0;
        while ((j < 3))
        {
          {
            var k = 0;
            while ((k < 3))
            {
              {
                var l = 0;
                while ((l < 3))
                {
                  if ((((j + k) + l) <= a[i]))
                  {
                    dp[(i + 1)][j][k] = max(dp[(i + 1)][j][k], ((dp[i][k][l] + l) + (((a[i] - (((j + k) + l)))) / 3)));
                  }
                  l += 1;
                }
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
  write(dp[(m + 1)][0][0], "\n");
  return 0;
}

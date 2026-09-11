// Translated from solution.cpp.

var a: dynamic = cpp_array(1000005);

var dp: dynamic = cpp_array(3, 3, 1000005);

var inf: dynamic = 0x3f3f3f3f;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&d));
      a[d] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= (m + 1)))
    {
      {
        var j: dynamic = 0;
        while ((j < 3))
        {
          {
            var k: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= m))
    {
      {
        var j: dynamic = 0;
        while ((j < 3))
        {
          {
            var k: dynamic = 0;
            while ((k < 3))
            {
              {
                var l: dynamic = 0;
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

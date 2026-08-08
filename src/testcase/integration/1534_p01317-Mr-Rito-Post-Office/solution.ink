// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var r: dynamic;

var INF = 1e7;

var d = cpp_array(200, 200, 2);

var dp = cpp_array(200, 1001);

func main()
{
  while (cpp_comma(scanf("%d %d", (&n), (&m)), n))
  {
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < n))
          {
            d[0][i][j] = cpp_assign(d[1][i][j], "=", INF);
            j += 1;
          }
        }
        d[0][i][i] = cpp_assign(d[1][i][i], "=", 0);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < m))
      {
        var x: dynamic;
        var y: dynamic;
        var t: dynamic;
        var c: dynamic;
        scanf("%d %d %d %c", (&x), (&y), (&t), (&c));
        x -= 1;
        y -= 1;
        d[(c == cpp_char("L"))][x][y] = min(d[(c == cpp_char("L"))][x][y], t);
        d[(c == cpp_char("L"))][y][x] = min(d[(c == cpp_char("L"))][y][x], t);
        i += 1;
      }
    }
    {
      var k = 0;
      while ((k < n))
      {
        {
          var i = 0;
          while ((i < n))
          {
            {
              var j = 0;
              while ((j < n))
              {
                d[0][i][j] = min(d[0][i][j], (d[0][i][k] + d[0][k][j]));
                d[1][i][j] = min(d[1][i][j], (d[1][i][k] + d[1][k][j]));
                j += 1;
              }
            }
            i += 1;
          }
        }
        k += 1;
      }
    }
    var now: dynamic;
    var z: dynamic;
    scanf("%d", (&r));
    scanf("%d", (&now));
    now -= 1;
    fill((&dp[0][0]), (&dp[1001][0]), INF);
    dp[0][now] = 0;
    {
      var i = 0;
      while ((i < (r - 1)))
      {
        scanf("%d", (&z));
        z -= 1;
        {
          var j = 0;
          while ((j < n))
          {
            dp[(i + 1)][j] = min(dp[(i + 1)][j], (dp[i][j] + d[1][now][z]));
            {
              var k = 0;
              while ((k < n))
              {
                dp[(i + 1)][k] = min(dp[(i + 1)][k], (((dp[i][j] + d[1][now][j]) + d[0][j][k]) + d[1][k][z]));
                k += 1;
              }
            }
            j += 1;
          }
        }
        now = z;
        i += 1;
      }
    }
    printf("%d\n", (*min_element(dp[(r - 1)], dp[r])));
  }
}

// Translated from solution.cpp.

var N = 66;

var INF = 1000000009;

var n: dynamic;

var m: dynamic;

var q: dynamic;

var a = cpp_array(N, N, N);

var dp = cpp_array(N, N, N);

func main()
{
  read(n, m, q);
  {
    var i = 0;
    while ((i < m))
    {
      {
        var x = 1;
        while ((x <= n))
        {
          {
            var y = 1;
            while ((y <= n))
            {
              read(a[i][x][y]);
              y += 1;
            }
          }
          x += 1;
        }
      }
      {
        var z = 1;
        while ((z <= n))
        {
          {
            var x = 1;
            while ((x <= n))
            {
              {
                var y = 1;
                while ((y <= n))
                {
                  a[i][x][y] = min(a[i][x][y], (a[i][x][z] + a[i][z][y]));
                  y += 1;
                }
              }
              x += 1;
            }
          }
          z += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j < N))
        {
          {
            var k = 0;
            while ((k < N))
            {
              dp[i][j][k] = INF;
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
    var i = 0;
    while ((i < m))
    {
      {
        var x = 1;
        while ((x <= n))
        {
          {
            var y = 1;
            while ((y <= n))
            {
              dp[x][y][0] = min(a[i][x][y], dp[x][y][0]);
              y += 1;
            }
          }
          x += 1;
        }
      }
      i += 1;
    }
  }
  {
    var k = 0;
    while ((k < n))
    {
      {
        var x = 1;
        while ((x <= n))
        {
          {
            var y = 1;
            while ((y <= n))
            {
              {
                var hy = 1;
                while ((hy <= n))
                {
                  dp[x][hy][(k + 1)] = min(dp[x][hy][(k + 1)], (dp[x][y][k] + dp[y][hy][0]));
                  hy += 1;
                }
              }
              y += 1;
            }
          }
          x += 1;
        }
      }
      k += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var x: dynamic;
    var y: dynamic;
    var k: dynamic;
    read(x, y, k);
    k = min(k, n);
    write(dp[x][y][k], "\n");
  }
  return 0;
}

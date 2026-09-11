// Translated from solution.cpp.

var N: dynamic = 66;

var INF: dynamic = 1000000009;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N, N, N);

var dp: dynamic = cpp_array(N, N, N);

func main() -> dynamic
{
  read(n, m, q);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var x: dynamic = 1;
        while ((x <= n))
        {
          {
            var y: dynamic = 1;
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
        var z: dynamic = 1;
        while ((z <= n))
        {
          {
            var x: dynamic = 1;
            while ((x <= n))
            {
              {
                var y: dynamic = 1;
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
    var i: dynamic = 0;
    while ((i < N))
    {
      {
        var j: dynamic = 0;
        while ((j < N))
        {
          {
            var k: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var x: dynamic = 1;
        while ((x <= n))
        {
          {
            var y: dynamic = 1;
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
    var k: dynamic = 0;
    while ((k < n))
    {
      {
        var x: dynamic = 1;
        while ((x <= n))
        {
          {
            var y: dynamic = 1;
            while ((y <= n))
            {
              {
                var hy: dynamic = 1;
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
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    read(x, y, k);
    k = min(k, n);
    write(dp[x][y][k], "\n");
  }
  return 0;
}

// Translated from solution.cpp.

var c = cpp_array((50 + 5));

var f = cpp_array((50 + 5), (50 + 5), (50 + 5));

var C = cpp_array((50 + 5), (50 + 5));

var F = cpp_array(2, (50 + 5));

var p = cpp_array((50 + 5));

func rw(x: dynamic, y: dynamic)
{
  if (((cpp_assign(x, "+=", y)) >= 1000000007))
  {
    x -= 1000000007;
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var x: dynamic;
  var ans = 0;
  scanf("%d%d", (&n), (&m));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= n))
    {
      {
        C[i][0] = cpp_assign(j, "=", 1);
        while ((j <= i))
        {
          C[i][j] = (((C[(i - 1)][(j - 1)] + C[(i - 1)][j])) % 1000000007);
          j += 1;
        }
      }
      {
        j = 0;
        while ((j <= i))
        {
          rw(F[i][(j & 1)], C[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    p[0] = cpp_assign(i, "=", 1);
    while ((i <= n))
    {
      p[i] = (((p[(i - 1)] + p[(i - 1)])) % 1000000007);
      i += 1;
    }
  }
  f[0][0][0] = 1;
  {
    i = 0;
    while ((i < n))
    {
      {
        j = 0;
        while ((j <= i))
        {
          {
            k = 0;
            while ((k <= i))
            {
              if (f[i][j][k])
              {
                if ((c[(i + 1)] != 0))
                {
                  rw(f[(i + 1)][(j + 1)][k], (((((1 * f[i][j][k]) * F[k][0]) % 1000000007) * p[(i - k)]) % 1000000007));
                  rw(f[(i + 1)][j][k], (((((1 * f[i][j][k]) * F[k][1]) % 1000000007) * p[(i - k)]) % 1000000007));
                }
                if ((c[(i + 1)] != 1))
                {
                  rw(f[(i + 1)][j][(k + 1)], (((((1 * f[i][j][k]) * F[j][0]) % 1000000007) * p[(i - j)]) % 1000000007));
                  rw(f[(i + 1)][j][k], (((((1 * f[i][j][k]) * F[j][1]) % 1000000007) * p[(i - j)]) % 1000000007));
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
  {
    i = 0;
    while ((i <= n))
    {
      {
        j = 0;
        while ((j <= n))
        {
          if ((((((i + j)) & 1)) == m))
          {
            rw(ans, f[n][i][j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d", ans);
}

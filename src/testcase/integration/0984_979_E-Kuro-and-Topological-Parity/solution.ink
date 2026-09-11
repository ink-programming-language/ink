// Translated from solution.cpp.

var c: dynamic = cpp_array((50 + 5));

var f: dynamic = cpp_array((50 + 5), (50 + 5), (50 + 5));

var C: dynamic = cpp_array((50 + 5), (50 + 5));

var F: dynamic = cpp_array(2, (50 + 5));

var p: dynamic = cpp_array((50 + 5));

func rw(x: dynamic, y: dynamic) -> dynamic
{
  if (((cpp_assign(x, "+=", y)) >= 1000000007))
  {
    x -= 1000000007;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
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

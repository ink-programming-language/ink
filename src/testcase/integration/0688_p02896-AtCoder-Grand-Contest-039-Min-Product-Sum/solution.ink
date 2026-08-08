// Translated from solution.cpp.

var int_cpp = dynamic;

var N = 105;

var f = cpp_array(N, N, N);

var g = cpp_array(N, N);

var p = cpp_array(N, N);

var mo: dynamic;

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k, mo);
  f[0][0][0] = 1;
  {
    var i = 0;
    while ((i <= 100))
    {
      g[i][0] = 1;
      {
        var j = 1;
        while ((j <= i))
        {
          g[i][j] = (((g[(i - 1)][j] + g[(i - 1)][(j - 1)])) % mo);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= 100))
    {
      p[i][0] = 1;
      {
        var j = 1;
        while ((j <= 100))
        {
          p[i][j] = ((p[i][(j - 1)] * i) % mo);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var h = 1;
    while ((h <= k))
    {
      {
        var i = 0;
        while ((i <= n))
        {
          {
            var j = 0;
            while ((j <= m))
            {
              {
                var t = (((p[((k - h) + 1)][j] * (((((p[h][(m - j)] - p[(h - 1)][(m - j)]) + mo)) % mo)))) % mo);
                var s = 1;
                var l = i;
                while ((l <= n))
                {
                  (cpp_assign(f[h][l][j], "+=", (((((s * g[(n - i)][(l - i)]) % mo) * f[(h - 1)][i][j]) % mo)))) %= mo;
                  (cpp_assign(s, "*=", t)) %= mo;
                  l += 1;
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
        while ((i <= n))
        {
          {
            var j = m;
            while ((~j))
            {
              {
                var t = (((p[h][(n - i)] * (((((p[((k - h) + 1)][i] - p[(k - h)][i]) + mo)) % mo)))) % mo);
                var s = 1;
                var l = (j + 1);
                while ((l <= m))
                {
                  (cpp_assign(s, "*=", t)) %= mo;
                  (cpp_assign(f[h][i][l], "+=", (((((s * g[(m - j)][(l - j)]) % mo) * f[h][i][j]) % mo)))) %= mo;
                  l += 1;
                }
              }
              j -= 1;
            }
          }
          i += 1;
        }
      }
      h += 1;
    }
  }
  write(f[k][n][m], cpp_char("\n"));
  return 0;
}

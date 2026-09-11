// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

var N: dynamic = 105;

var f: dynamic = cpp_array(N, N, N);

var g: dynamic = cpp_array(N, N);

var p: dynamic = cpp_array(N, N);

var mo: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k, mo);
  f[0][0][0] = 1;
  {
    var i: dynamic = 0;
    while ((i <= 100))
    {
      g[i][0] = 1;
      {
        var j: dynamic = 1;
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
    var i: dynamic = 0;
    while ((i <= 100))
    {
      p[i][0] = 1;
      {
        var j: dynamic = 1;
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
    var h: dynamic = 1;
    while ((h <= k))
    {
      {
        var i: dynamic = 0;
        while ((i <= n))
        {
          {
            var j: dynamic = 0;
            while ((j <= m))
            {
              {
                var t: dynamic = (((p[((k - h) + 1)][j] * (((((p[h][(m - j)] - p[(h - 1)][(m - j)]) + mo)) % mo)))) % mo);
                var s: dynamic = 1;
                var l: dynamic = i;
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
        var i: dynamic = 0;
        while ((i <= n))
        {
          {
            var j: dynamic = m;
            while ((~j))
            {
              {
                var t: dynamic = (((p[h][(n - i)] * (((((p[((k - h) + 1)][i] - p[(k - h)][i]) + mo)) % mo)))) % mo);
                var s: dynamic = 1;
                var l: dynamic = (j + 1);
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

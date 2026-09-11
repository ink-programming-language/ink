// Translated from solution.cpp.

var f: dynamic = cpp_array(505, 505);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      f[0][i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          {
            var k: dynamic = 0;
            while ((k < j))
            {
              (cpp_assign(f[j][i], "+=", (f[k][(i - 1)] * f[((j - k) - 1)][(i - 1)])));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld", (f[n][n] - f[n][(k - 1)]));
  return 0;
}

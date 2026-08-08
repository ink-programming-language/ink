// Translated from solution.cpp.

var f = cpp_array(505, 505);

func main()
{
  var n: dynamic;
  var k: dynamic;
  scanf("%d%d", (&n), (&k));
  {
    var i = 0;
    while ((i <= n))
    {
      f[0][i] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          {
            var k = 0;
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

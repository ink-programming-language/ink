// Translated from solution.cpp.

var N = 100;

func main()
{
  var n: dynamic;
  var p = cpp_array((N + 1));
  var m = cpp_array((N + 1), (N + 1));
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(p[(i - 1)], p[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      m[i][i] = 0;
      i += 1;
    }
  }
  {
    var l = 2;
    while ((l <= n))
    {
      {
        var i = 1;
        while ((i <= ((n - l) + 1)))
        {
          var j = ((i + l) - 1);
          m[i][j] = ((1 << 21));
          {
            var k = i;
            while ((k <= (j - 1)))
            {
              m[i][j] = min(m[i][j], ((m[i][k] + m[(k + 1)][j]) + ((p[(i - 1)] * p[k]) * p[j])));
              k += 1;
            }
          }
          i += 1;
        }
      }
      l += 1;
    }
  }
  write(m[1][n], "\n");
}

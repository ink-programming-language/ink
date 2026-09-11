// Translated from solution.cpp.

var N: dynamic = 100;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_array((N + 1));
  var m: dynamic = cpp_array((N + 1), (N + 1));
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(p[(i - 1)], p[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      m[i][i] = 0;
      i += 1;
    }
  }
  {
    var l: dynamic = 2;
    while ((l <= n))
    {
      {
        var i: dynamic = 1;
        while ((i <= ((n - l) + 1)))
        {
          var j: dynamic = ((i + l) - 1);
          m[i][j] = ((1 << 21));
          {
            var k: dynamic = i;
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

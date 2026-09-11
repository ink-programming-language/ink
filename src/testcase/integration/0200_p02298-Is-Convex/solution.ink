// Translated from solution.cpp.

var x: dynamic = cpp_array(10010);

var y: dynamic = cpp_array(10010);

var ans: dynamic = cpp_uninitialized();

var pd: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(x[i], y[i]);
      x[(i + n)] = x[i];
      y[(i + n)] = y[i];
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((j <= (n - 1)))
    {
      var xx: dynamic = x[j];
      var yy: dynamic = y[j];
      {
        var i: dynamic = 1;
        while ((i <= (2 * n)))
        {
          x[i] -= xx;
          y[i] -= yy;
          i += 1;
        }
      }
      {
        var i: dynamic = ((3 + j) - 1);
        while ((i <= ((n + j) - 1)))
        {
          if ((((x[(i - 1)] * y[i]) - (x[i] * y[(i - 1)])) < 0))
          {
            pd = 1;
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= (2 * n)))
        {
          x[i] -= xx;
          y[i] -= yy;
          i += 1;
        }
      }
      j += 1;
    }
  }
  write(((pd ^ 1)), "\n");
  return 0;
}

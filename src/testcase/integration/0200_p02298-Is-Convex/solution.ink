// Translated from solution.cpp.

var x = cpp_array(10010);

var y = cpp_array(10010);

var ans: dynamic;

var pd: dynamic;

var n: dynamic;

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(x[i], y[i]);
      x[(i + n)] = x[i];
      y[(i + n)] = y[i];
      i += 1;
    }
  }
  {
    var j = 1;
    while ((j <= (n - 1)))
    {
      var xx = x[j];
      var yy = y[j];
      {
        var i = 1;
        while ((i <= (2 * n)))
        {
          x[i] -= xx;
          y[i] -= yy;
          i += 1;
        }
      }
      {
        var i = ((3 + j) - 1);
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
        var i = 1;
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

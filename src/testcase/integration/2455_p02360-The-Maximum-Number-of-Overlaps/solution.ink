// Translated from solution.cpp.

var SZ = cpp_expression("#inc");

func MAX(X: dynamic, Y: dynamic)
{
  return cpp_expression("#include <iostrea");
}

func main(argument_0: dynamic)
{
  var n: dynamic;
  var d = cpp_array((SZ + 1), (SZ + 1));
  var ans: dynamic;
  read(n);
  {
    var i = 0;
    while ((i <= SZ))
    {
      {
        var j = 0;
        while ((j <= SZ))
        {
          d[i][j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var x0: dynamic;
      var y0: dynamic;
      var x1: dynamic;
      var y1: dynamic;
      read(x0, y0, x1, y1);
      d[x0][y0] += 1;
      d[x1][y1] += 1;
      d[x0][y1] -= 1;
      d[x1][y0] -= 1;
      i += 1;
    }
  }
  {
    var x = 0;
    while ((x <= SZ))
    {
      {
        var y = 1;
        while ((y <= SZ))
        {
          d[x][y] += d[x][(y - 1)];
          y += 1;
        }
      }
      x += 1;
    }
  }
  ans = d[0][0];
  {
    var y = 0;
    while ((y <= SZ))
    {
      ans = MAX(ans, d[0][y]);
      {
        var x = 1;
        while ((x <= SZ))
        {
          d[x][y] += d[(x - 1)][y];
          ans = MAX(ans, d[x][y]);
          x += 1;
        }
      }
      y += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

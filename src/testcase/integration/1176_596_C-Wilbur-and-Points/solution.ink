// Translated from solution.cpp.

var MAXN: dynamic = 100000;

var MAXNUM: dynamic = 100000;

var n: dynamic = cpp_uninitialized();

var max_x: dynamic = cpp_array((MAXNUM + 1));

var max_y: dynamic = cpp_uninitialized();

var w: dynamic = cpp_array(MAXN);

var res: dynamic = cpp_array(2, MAXN);

func lookup(t: dynamic) -> dynamic
{
  var p: dynamic = 0;
  var q: dynamic = max_y;
  while ((p < q))
  {
    var y: dynamic = (((p + q)) / 2);
    if (((y - max_x[y]) < t))
    {
      p = (y + 1);
    } else
    {
      q = y;
    }
  }
  if ((p > q))
  {
    return -1;
  }
  if (((p - max_x[p]) != t))
  {
    return -1;
  }
  if (((p < max_y) && (max_x[p] == max_x[(p + 1)])))
  {
    return -1;
  }
  return p;
}

func main() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= MAXNUM))
    {
      max_x[i] = -1;
      i += 1;
    }
  }
  max_y = -1;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      if ((x > max_x[y]))
      {
        max_x[y] = x;
      }
      if ((y > max_y))
      {
        max_y = y;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(w[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      var y: dynamic = lookup(w[i]);
      if ((y == -1))
      {
        write("NO", "\n");
        return 0;
      }
      res[i][0] = cpp_update(max_x[y], "--");
      res[i][1] = y;
      i -= 1;
    }
  }
  write("YES", "\n");
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(res[i][0], " ", res[i][1], "\n");
      i += 1;
    }
  }
  return 0;
}

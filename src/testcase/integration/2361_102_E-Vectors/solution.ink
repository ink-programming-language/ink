// Translated from solution.cpp.

var p: dynamic;

var q: dynamic;

var x: dynamic;

var y: dynamic;

func possible(x: dynamic, y: dynamic)
{
  var bm = (((1 * p) * p) + ((1 * q) * q));
  if ((bm == 0))
  {
    if (((x || y)))
    {
      return false;
    } else
    {
      return true;
    }
  } else if (((((((((-1 * p) * x) - ((1 * q) * y))) % bm)) || ((((((-1 * q) * x) + ((1 * p) * y))) % bm)))))
  {
    return false;
  } else
  {
    return true;
  }
}

func main()
{
  var ans = false;
  var ax: dynamic;
  var ay: dynamic;
  read(ax, ay, x, y, p, q);
  var i: dynamic;
  var tmp: dynamic;
  {
    i = 0;
    while ((i < 4))
    {
      x -= ax;
      y -= ay;
      ans = (ans || ((possible(x, y) || possible((-y), x))));
      x += ax;
      y += ay;
      tmp = ax;
      ax = ay;
      ay = (-tmp);
      i += 1;
    }
  }
  if (ans)
  {
    write("YES");
  } else
  {
    write("NO");
  }
  return 0;
}

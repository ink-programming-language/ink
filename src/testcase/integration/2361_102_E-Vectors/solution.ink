// Translated from solution.cpp.

var p: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

func possible(x: dynamic, y: dynamic) -> dynamic
{
  var bm: dynamic = (((1 * p) * p) + ((1 * q) * q));
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

func main() -> dynamic
{
  var ans: dynamic = false;
  var ax: dynamic = cpp_uninitialized();
  var ay: dynamic = cpp_uninitialized();
  read(ax, ay, x, y, p, q);
  var i: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
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

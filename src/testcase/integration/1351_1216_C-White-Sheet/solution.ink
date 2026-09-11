// Translated from solution.cpp.

var INFTY: dynamic = 20000000;

var MAX: dynamic = 500100;

var MOD: dynamic = 10000000;

func coutTab(tab: dynamic, n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(tab[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func coutVec(tab: dynamic) -> dynamic
{
  for (var t: dynamic in tab)
  {
    write(t, " ");
  }
  write("\n");
}

class Square
{
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  func Square() -> dynamic
  {
      self->x1 = cpp_construct(0);
      self->y1 = cpp_construct(0);
      self->x2 = cpp_construct(0);
      self->y2 = cpp_construct(0);
    }
  func Square(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic) -> dynamic
  {
      self->x1 = cpp_construct(x1);
      self->y1 = cpp_construct(y1);
      self->x2 = cpp_construct(x2);
      self->y2 = cpp_construct(y2);
    }
}

func field(a: dynamic) -> dynamic
{
  return (((a.x2 - a.x1)) * ((a.y2 - a.y1)));
}

func intersect(a: dynamic, b: dynamic) -> dynamic
{
  var x1: dynamic = max(a.x1, b.x1);
  var x2: dynamic = min(a.x2, b.x2);
  var y1: dynamic = max(a.y1, b.y1);
  var y2: dynamic = min(a.y2, b.y2);
  if (((x1 > x2) || (y1 > y2)))
  {
    return [false, Square()];
  }
  return [true, Square(x1, y1, x2, y2)];
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  var w: dynamic = cpp_uninitialized();
  var b1: dynamic = cpp_uninitialized();
  var b2: dynamic = cpp_uninitialized();
  read(w.x1, w.y1, w.x2, w.y2);
  read(b1.x1, b1.y1, b1.x2, b1.y2);
  read(b2.x1, b2.y1, b2.x2, b2.y2);
  var i1: dynamic = intersect(w, b1);
  var i2: dynamic = intersect(w, b2);
  if ((i1.first && i2.first))
  {
    write(( ((((field(i1.second) + field(i2.second)) - field(intersect(i1.second, i2.second).second)) == field(w))) ? "NO" : "YES"), "\n");
  } else if (i1.first)
  {
    write(( ((field(i1.second) == field(w))) ? "NO" : "YES"), "\n");
  } else if (i2.first)
  {
    write(( ((field(i2.second) == field(w))) ? "NO" : "YES"), "\n");
  } else
  {
    write("YES", "\n");
  }
}

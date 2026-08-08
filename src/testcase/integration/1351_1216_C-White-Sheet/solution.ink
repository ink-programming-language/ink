// Translated from solution.cpp.

var INFTY = 20000000;

var MAX = 500100;

var MOD = 10000000;

func coutTab(tab: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      write(tab[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func coutVec(tab: dynamic)
{
  for (var t in tab)
  {
    write(t, " ");
  }
  write("\n");
}

class Square
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  func Square()
  {
      this->x1 = cpp_construct(0);
      this->y1 = cpp_construct(0);
      this->x2 = cpp_construct(0);
      this->y2 = cpp_construct(0);
    }
  func Square(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
  {
      this->x1 = cpp_construct(x1);
      this->y1 = cpp_construct(y1);
      this->x2 = cpp_construct(x2);
      this->y2 = cpp_construct(y2);
    }
}

func field(a: dynamic)
{
  return (((a.x2 - a.x1)) * ((a.y2 - a.y1)));
}

func intersect(a: dynamic, b: dynamic)
{
  var x1 = max(a.x1, b.x1);
  var x2 = min(a.x2, b.x2);
  var y1 = max(a.y1, b.y1);
  var y2 = min(a.y2, b.y2);
  if (((x1 > x2) || (y1 > y2)))
  {
    return [false, Square()];
  }
  return [true, Square(x1, y1, x2, y2)];
}

func main()
{
  ios_base.sync_with_stdio(0);
  var w: dynamic;
  var b1: dynamic;
  var b2: dynamic;
  read(w.x1, w.y1, w.x2, w.y2);
  read(b1.x1, b1.y1, b1.x2, b1.y2);
  read(b2.x1, b2.y1, b2.x2, b2.y2);
  var i1 = intersect(w, b1);
  var i2 = intersect(w, b2);
  if ((i1.first && i2.first))
  {
    write((if ((((field(i1.second) + field(i2.second)) - field(intersect(i1.second, i2.second).second)) == field(w))) "NO" else "YES"), "\n");
  } else if (i1.first)
  {
    write((if ((field(i1.second) == field(w))) "NO" else "YES"), "\n");
  } else if (i2.first)
  {
    write((if ((field(i2.second) == field(w))) "NO" else "YES"), "\n");
  } else
  {
    write("YES", "\n");
  }
}

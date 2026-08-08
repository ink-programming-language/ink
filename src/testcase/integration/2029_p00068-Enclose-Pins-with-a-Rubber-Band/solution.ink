// Translated from solution.cpp.

class Point
{
}

class Sentinel
{
}

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point(a: dynamic = [0], b: dynamic = [0])
  {
      this->x = cpp_construct(a);
      this->y = cpp_construct(b);
    }
  func operator_less(other: dynamic)
  {
      return (((x < other.x)) || ((((x == other.x)) && ((y < other.y)))));
    }
  func operator_greater(other: dynamic)
  {
      return (other < (*this));
    }
  func operator_multiply(other: dynamic)
  {
      return ((x * other.y) - (y * other.x));
    }
  func inspect()
  {
      write(x, "-", y, "\n");
    }
  func operator_subtract(other: dynamic)
  {
      return Point((x - other.x), (y - other.y));
    }
}

class Sentinel
{
  func Sentinel()
  {
      this->Point = cpp_construct(10000, 10000);
    }
}

func sort(vec: dynamic, left: dynamic, right: dynamic)
{
  if ((left == right))
  {
    return [vec.at(left), Sentinel()];
  } else
  {
    var l = sort(vec, left, (left + (((right - left)) / 2)));
    var r = sort(vec, ((left + 1) + (((right - left)) / 2)), right);
    var res: dynamic;
    for (var a in l)
    {
      while ((r.front() < a))
      {
        res.push_back(r.front());
        r.pop_front();
      }
      res.push_back(a);
    }
    return res;
  }
}

func sub(a: dynamic, b: dynamic, c: dynamic)
{
  return (((a - b)) * ((b - c)));
}

func main()
{
  var n: dynamic;
  read(n);
  while ((n != 0))
  {
    for (var v in vec)
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%lf,%lf", (&x), (&y));
      v = Point(x, y);
    }
    var list = sort(vec, 0, (n - 1));
    list.pop_back();
    var a = cpp_construct((n + 2));
    var b = cpp_construct((n + 2));
    a.at(0) = list.front();
    a.at(1) = list.front();
    b.at(0) = list.front();
    b.at(1) = list.front();
    var i = 1;
    var j = 1;
    for (var p in list)
    {
      while ((sub(a.at((i - 1)), a.at(i), p) > 0))
      {
        i -= 1;
      }
      a.at(cpp_update(i, "++")) = p;
      while ((sub(b.at((j - 1)), b.at(j), p) < 0))
      {
        j -= 1;
      }
      b.at(cpp_update(j, "++")) = p;
    }
    write((((n - i) - j) + 4), "\n");
    read(n);
  }
  return 0;
}

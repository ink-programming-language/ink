// Translated from solution.cpp.

class Point
{
}

class Sentinel
{
}

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point(a: dynamic = [0], b: dynamic = [0]) -> dynamic
  {
      self->x = cpp_construct(a);
      self->y = cpp_construct(b);
    }
  func operator_less(other: dynamic) -> dynamic
  {
      return (((x < other.x)) || ((((x == other.x)) && ((y < other.y)))));
    }
  func operator_greater(other: dynamic) -> dynamic
  {
      return (other < (*self));
    }
  func operator_multiply(other: dynamic) -> dynamic
  {
      return ((x * other.y) - (y * other.x));
    }
  func inspect() -> dynamic
  {
      write(x, "-", y, "\n");
    }
  func operator_subtract(other: dynamic) -> dynamic
  {
      return Point((x - other.x), (y - other.y));
    }
}

class Sentinel
{
  func Sentinel() -> dynamic
  {
      self->Point = cpp_construct(10000, 10000);
    }
}

func sort(vec: dynamic, left: dynamic, right: dynamic) -> dynamic
{
  if ((left == right))
  {
    return [vec.at(left), Sentinel()];
  } else
  {
    var l: dynamic = sort(vec, left, (left + (((right - left)) / 2)));
    var r: dynamic = sort(vec, ((left + 1) + (((right - left)) / 2)), right);
    var res: dynamic = cpp_uninitialized();
    for (var a: dynamic in l)
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

func sub(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  return (((a - b)) * ((b - c)));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  while ((n != 0))
  {
    for (var v: dynamic in vec)
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%lf,%lf", (&x), (&y));
      v = Point(x, y);
    }
    var list: dynamic = sort(vec, 0, (n - 1));
    list.pop_back();
    var a: dynamic = cpp_construct((n + 2));
    var b: dynamic = cpp_construct((n + 2));
    a.at(0) = list.front();
    a.at(1) = list.front();
    b.at(0) = list.front();
    b.at(1) = list.front();
    var i: dynamic = 1;
    var j: dynamic = 1;
    for (var p: dynamic in list)
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

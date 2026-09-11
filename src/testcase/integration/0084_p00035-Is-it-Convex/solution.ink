// Translated from solution.cpp.

var EPS: dynamic = cpp_expression("#inc");

func pb(a: dynamic) -> dynamic
{
  cpp_macro("push_back(a);");
}

var ID_CCW: dynamic = cpp_expression("#");

var ID_CW: dynamic = cpp_expression("#");

var ID_GO: dynamic = cpp_expression("#");

var ID_BACK: dynamic = cpp_expression("#");

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point() -> dynamic
  {
    }
  func Point(xx: dynamic, yy: dynamic) -> dynamic
  {
      self->x = cpp_construct(xx);
      self->y = cpp_construct(yy);
    }
}

func operator_subtract_assign(a: dynamic, b: dynamic) -> dynamic
{
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  var rotdir: dynamic = cross(b, c);
  if ((rotdir > EPS))
  {
    return ID_CCW;
  }
  if ((rotdir < (-EPS)))
  {
    return ID_CW;
  }
  if ((dot(b, c) > EPS))
  {
    return ID_GO;
  }
  return ID_BACK;
}

func main() -> dynamic
{
  var p: dynamic = cpp_array(4);
  while (1)
  {
    if ((scanf("%lf,%lf", (&p[0].x), (&p[0].y)) == EOF))
    {
      break;
    }
    {
      var i: dynamic = 1;
      while ((i < 4))
      {
        scanf(",%lf,%lf", (&p[i].x), (&p[i].y));
        i += 1;
      }
    }
    var b1: dynamic = true;
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        if ((ccw(p[i], p[(((i + 1)) % 4)], p[(((i + 2)) % 4)]) != ID_CCW))
        {
          b1 = false;
        }
        i += 1;
      }
    }
    var b2: dynamic = true;
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        if ((ccw(p[i], p[(((i + 1)) % 4)], p[(((i + 2)) % 4)]) != ID_CW))
        {
          b2 = false;
        }
        i += 1;
      }
    }
    puts( ((b1 || b2)) ? "YES" : "NO");
  }
  return 0;
}

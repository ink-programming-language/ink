// Translated from solution.cpp.

var EPS = cpp_expression("#inc");

func pb(a: dynamic)
{
  cpp_macro("push_back(a);");
}

var ID_CCW = cpp_expression("#");

var ID_CW = cpp_expression("#");

var ID_GO = cpp_expression("#");

var ID_BACK = cpp_expression("#");

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point()
  {
    }
  func Point(xx: dynamic, yy: dynamic)
  {
      this->x = cpp_construct(xx);
      this->y = cpp_construct(yy);
    }
}

func operator_subtract_assign(a: dynamic, b: dynamic)
{
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

func dot(a: dynamic, b: dynamic)
{
  return ((a.x * b.x) + (a.y * b.y));
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  b -= a;
  c -= a;
  var rotdir = cross(b, c);
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

func main()
{
  var p = cpp_array(4);
  while (1)
  {
    if ((scanf("%lf,%lf", (&p[0].x), (&p[0].y)) == EOF))
    {
      break;
    }
    {
      var i = 1;
      while ((i < 4))
      {
        scanf(",%lf,%lf", (&p[i].x), (&p[i].y));
        i += 1;
      }
    }
    var b1 = true;
    {
      var i = 0;
      while ((i < 4))
      {
        if ((ccw(p[i], p[(((i + 1)) % 4)], p[(((i + 2)) % 4)]) != ID_CCW))
        {
          b1 = false;
        }
        i += 1;
      }
    }
    var b2 = true;
    {
      var i = 0;
      while ((i < 4))
      {
        if ((ccw(p[i], p[(((i + 1)) % 4)], p[(((i + 2)) % 4)]) != ID_CW))
        {
          b2 = false;
        }
        i += 1;
      }
    }
    puts(if ((b1 || b2)) "YES" else "NO");
  }
  return 0;
}

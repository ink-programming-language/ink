// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var EPS = 1e-8;

var PI = acos(-1);

class point
{
  var x: dynamic;
  var y: dynamic;
  func point()
  {
      this->x = cpp_construct(0);
      this->y = cpp_construct(0);
    }
  func point(x: dynamic, y: dynamic)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_subtract(a: dynamic)
  {
      return point((x - a.x), (y - a.y));
    }
}

func cross(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func arg(a: dynamic)
{
  var t = atan2(a.y, a.x);
  return if ((t < 0)) (t + (2 * PI)) else t;
}

class line
{
  var a: dynamic;
  var b: dynamic;
  func line()
  {
    }
  func line(a: dynamic, b: dynamic)
  {
      this->a = cpp_construct(a);
      this->b = cpp_construct(b);
    }
}

enum cpp_enum_1
{
  CCW = 1,
  CW = -1,
  ON = 0
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  var rdir = cross((b - a), (c - a));
  if ((rdir > 0))
  {
    return CCW;
  }
  if ((rdir < 0))
  {
    return CW;
  }
  return ON;
}

func dist(a: dynamic, b: dynamic)
{
  return sqrt(((((a.x - b.x)) * ((a.x - b.x))) + (((a.y - b.y)) * ((a.y - b.y)))));
}

var g = 9.8;

func main()
{
  var n: dynamic;
  var v: dynamic;
  scanf("%d%Lf", (&n), (&v));
  var p = cpp_array(10000);
  rep(i, n);
  scanf("%Lf%Lf", (&p[i].x), (&p[i].y));
  var a = cpp_array(9999);
  var b = cpp_array(9999);
  rep(i, (n - 1));
  {
    a[i] = (((p[(i + 1)].y - p[i].y)) / ((p[(i + 1)].x - p[i].x)));
    b[i] = (p[i].y - (a[i] * p[i].x));
  }
  var ans = dist(p[0], p[1]);
  {
    var i = 1;
    while ((i < (n - 1)))
    {
      if ((ccw(p[(i - 1)], p[i], p[(i + 1)]) != CW))
      {
        ans += dist(p[i], p[(i + 1)]);
        i += 1;
      } else
      {
        var vx = (v * cos(arg((p[i] - p[(i - 1)]))));
        var vy = (v * sin(arg((p[i] - p[(i - 1)]))));
        var x0 = p[i].x;
        var y0 = p[i].y;
        {
          while (((i + 1) < n))
          {
            var A = ((-g) / 2);
            var B = (vy - (a[i] * vx));
            var C = ((y0 - (a[i] * x0)) - b[i]);
            if ((((B * B) - ((4 * A) * C)) < 0))
            {
              i += 1;
              continue;
            }
            var t0 = ((((-B) + sqrt(((B * B) - ((4 * A) * C))))) / ((2 * A)));
            var t1 = ((((-B) - sqrt(((B * B) - ((4 * A) * C))))) / ((2 * A)));
            var xx: dynamic;
            xx = (x0 + (vx * t0));
            if (((p[i].x < xx) && (xx < p[(i + 1)].x)))
            {
              var q = cpp_construct(xx, ((((((-g) / 2) * t0) * t0) + (vy * t0)) + y0));
              ans += dist(q, p[(i + 1)]);
              break;
            }
            xx = (x0 + (vx * t1));
            if (((p[i].x < xx) && (xx < p[(i + 1)].x)))
            {
              var q = cpp_construct(xx, ((((((-g) / 2) * t1) * t1) + (vy * t1)) + y0));
              ans += dist(q, p[(i + 1)]);
              break;
            }
            i += 1;
          }
        }
        i += 1;
      }
    }
  }
  printf("%.15Lf\n", ans);
  return 0;
}

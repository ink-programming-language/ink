// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var EPS: dynamic = 1e-8;

var PI: dynamic = acos(-1);

class point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func point() -> dynamic
  {
      self->x = cpp_construct(0);
      self->y = cpp_construct(0);
    }
  func point(x: dynamic, y: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func operator_subtract(a: dynamic) -> dynamic
  {
      return point((x - a.x), (y - a.y));
    }
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func arg(a: dynamic) -> dynamic
{
  var t: dynamic = atan2(a.y, a.x);
  return  ((t < 0)) ? (t + (2 * PI)) : t;
}

class line
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func line() -> dynamic
  {
    }
  func line(a: dynamic, b: dynamic) -> dynamic
  {
      self->a = cpp_construct(a);
      self->b = cpp_construct(b);
    }
}

enum cpp_enum_1
{
  enum_field CCW = 1;
  enum_field CW = -1;
  enum_field ON = 0;
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var rdir: dynamic = cross((b - a), (c - a));
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

func dist(a: dynamic, b: dynamic) -> dynamic
{
  return sqrt(((((a.x - b.x)) * ((a.x - b.x))) + (((a.y - b.y)) * ((a.y - b.y)))));
}

var g: dynamic = 9.8;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  scanf("%d%Lf", (&n), (&v));
  var p: dynamic = cpp_array(10000);
  rep(i, n);
  scanf("%Lf%Lf", (&p[i].x), (&p[i].y));
  var a: dynamic = cpp_array(9999);
  var b: dynamic = cpp_array(9999);
  rep(i, (n - 1));
  {
    a[i] = (((p[(i + 1)].y - p[i].y)) / ((p[(i + 1)].x - p[i].x)));
    b[i] = (p[i].y - (a[i] * p[i].x));
  }
  var ans: dynamic = dist(p[0], p[1]);
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      if ((ccw(p[(i - 1)], p[i], p[(i + 1)]) != CW))
      {
        ans += dist(p[i], p[(i + 1)]);
        i += 1;
      } else
      {
        var vx: dynamic = (v * cos(arg((p[i] - p[(i - 1)]))));
        var vy: dynamic = (v * sin(arg((p[i] - p[(i - 1)]))));
        var x0: dynamic = p[i].x;
        var y0: dynamic = p[i].y;
        {
          while (((i + 1) < n))
          {
            var A: dynamic = ((-g) / 2);
            var B: dynamic = (vy - (a[i] * vx));
            var C: dynamic = ((y0 - (a[i] * x0)) - b[i]);
            if ((((B * B) - ((4 * A) * C)) < 0))
            {
              i += 1;
              continue;
            }
            var t0: dynamic = ((((-B) + sqrt(((B * B) - ((4 * A) * C))))) / ((2 * A)));
            var t1: dynamic = ((((-B) - sqrt(((B * B) - ((4 * A) * C))))) / ((2 * A)));
            var xx: dynamic = cpp_uninitialized();
            xx = (x0 + (vx * t0));
            if (((p[i].x < xx) && (xx < p[(i + 1)].x)))
            {
              var q: dynamic = cpp_construct(xx, ((((((-g) / 2) * t0) * t0) + (vy * t0)) + y0));
              ans += dist(q, p[(i + 1)]);
              break;
            }
            xx = (x0 + (vx * t1));
            if (((p[i].x < xx) && (xx < p[(i + 1)].x)))
            {
              var q: dynamic = cpp_construct(xx, ((((((-g) / 2) * t1) * t1) + (vy * t1)) + y0));
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

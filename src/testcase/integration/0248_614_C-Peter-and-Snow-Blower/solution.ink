// Translated from solution.cpp.

var pi: dynamic = acos(-1.0);

class node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(100005);

var b: dynamic = cpp_uninitialized();

func fun(a: dynamic) -> dynamic
{
  return sqrt(((((b.x - a.x)) * ((b.x - a.x))) + (((b.y - a.y)) * ((b.y - a.y)))));
}

func fun(a: dynamic, b: dynamic) -> dynamic
{
  return sqrt(((((b.x - a.x)) * ((b.x - a.x))) + (((b.y - a.y)) * ((b.y - a.y)))));
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  while ((~scanf("%d%lf%lf", (&n), (&b.x), (&b.y))))
  {
    {
      i = 0;
      while ((i < n))
      {
        scanf("%lf%lf", (&a[i].x), (&a[i].y));
        i += 1;
      }
    }
    a[n].x = a[0].x;
    a[n].y = a[0].y;
    var r_min: dynamic = 1e18;
    var r_max: dynamic = 0;
    var t_r1: dynamic = cpp_uninitialized();
    var t_r2: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < n))
      {
        t_r1 = fun(a[i]);
        t_r2 = fun(a[(i + 1)]);
        r_max = max(r_max, t_r1);
        r_max = max(r_max, t_r2);
        var t: dynamic = fun(a[i], a[(i + 1)]);
        if ((((t_r1 * t_r1) + (t * t)) < (t_r2 * t_r2)))
        {
          r_min = min(r_min, t_r1);
        } else if ((((t_r2 * t_r2) + (t * t)) < (t_r1 * t_r1)))
        {
          r_min = min(r_min, t_r2);
        } else
        {
          var p: dynamic = ((((t + t_r1) + t_r2)) / 2);
          var s: dynamic = sqrt((((p * ((p - t))) * ((p - t_r1))) * ((p - t_r2))));
          var r: dynamic = ((2 * s) / t);
          r_min = min(r_min, r);
        }
        i += 1;
      }
    }
    printf("%.12f\n", (pi * (((r_max * r_max) - (r_min * r_min)))));
  }
  return 0;
}

// Translated from solution.cpp.

var pi = acos(-1.0);

class node
{
  var x: dynamic;
  var y: dynamic;
}

var a = cpp_array(100005);

var b: dynamic;

func fun(a: dynamic)
{
  return sqrt(((((b.x - a.x)) * ((b.x - a.x))) + (((b.y - a.y)) * ((b.y - a.y)))));
}

func fun(a: dynamic, b: dynamic)
{
  return sqrt(((((b.x - a.x)) * ((b.x - a.x))) + (((b.y - a.y)) * ((b.y - a.y)))));
}

func main()
{
  var i: dynamic;
  var n: dynamic;
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
    var r_min = 1e18;
    var r_max = 0;
    var t_r1: dynamic;
    var t_r2: dynamic;
    {
      i = 0;
      while ((i < n))
      {
        t_r1 = fun(a[i]);
        t_r2 = fun(a[(i + 1)]);
        r_max = max(r_max, t_r1);
        r_max = max(r_max, t_r2);
        var t = fun(a[i], a[(i + 1)]);
        if ((((t_r1 * t_r1) + (t * t)) < (t_r2 * t_r2)))
        {
          r_min = min(r_min, t_r1);
        } else if ((((t_r2 * t_r2) + (t * t)) < (t_r1 * t_r1)))
        {
          r_min = min(r_min, t_r2);
        } else
        {
          var p = ((((t + t_r1) + t_r2)) / 2);
          var s = sqrt((((p * ((p - t))) * ((p - t_r1))) * ((p - t_r2))));
          var r = ((2 * s) / t);
          r_min = min(r_min, r);
        }
        i += 1;
      }
    }
    printf("%.12f\n", (pi * (((r_max * r_max) - (r_min * r_min)))));
  }
  return 0;
}

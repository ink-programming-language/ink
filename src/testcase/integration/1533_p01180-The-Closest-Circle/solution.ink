// Translated from solution.cpp.

var INF = cpp_expression("#inc");

var n: dynamic;

class P
{
  var x: dynamic;
  var y: dynamic;
  var r: dynamic;
  func P()
  {
    }
  func P(xx: dynamic, yy: dynamic, rr: dynamic)
  {
      x = xx;
      y = yy;
      r = rr;
    }
  func operator_less(p1: dynamic)
  {
      return (x < p1.x);
    }
}

func compare_y(a: dynamic, b: dynamic)
{
  return (a.y < b.y);
}

func closest_pair(a: dynamic, n: dynamic)
{
  if ((n <= 1))
  {
    return INF;
  }
  var m = (n / 2);
  var x = a[m].x;
  var r = a[m].r;
  var d = min(closest_pair(a, m), closest_pair((a + m), (n - m)));
  inplace_merge(a, (a + m), (a + n), compare_y);
  var b: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((fabs((a[i].x - x)) - a[i].r) - r) >= d))
      {
        i += 1;
        continue;
      }
      {
        var j = 0;
        while ((j < b.size()))
        {
          var dx = (a[i].x - b[((b.size() - j) - 1)].x);
          var dy = (a[i].y - b[((b.size() - j) - 1)].y);
          if ((dy >= ((d + r) + a[i].r)))
          {
            break;
          }
          d = min(d, ((sqrt(((dx * dx) + (dy * dy))) - b[((b.size() - j) - 1)].r) - a[i].r));
          j += 1;
        }
      }
      b.push_back(a[i]);
      r = max(a[i].r, r);
      i += 1;
    }
  }
  return d;
}

var p = cpp_array(100001);

func main(argument_0: dynamic)
{
  while (1)
  {
    scanf("%d", (&n));
    if ((n == 0))
    {
      break;
    }
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%lf %lf %lf", (&p[i].r), (&p[i].x), (&p[i].y));
        i += 1;
      }
    }
    sort(p, (p + n));
    printf("%.9f\n", closest_pair(p, n));
  }
  return 0;
}

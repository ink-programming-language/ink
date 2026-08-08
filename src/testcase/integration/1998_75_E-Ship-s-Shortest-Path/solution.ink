// Translated from solution.cpp.

var eps = 1e-4;

class Point
{
  var x: dynamic;
  var y: dynamic;
  func Point()
  {
      x = cpp_assign(y, "=", 0);
    }
  func Point(xx: dynamic, yy: dynamic)
  {
      x = xx;
      y = yy;
    }
  func operator_add(b: dynamic)
  {
      return Point((x + b.x), (y + b.y));
    }
  func operator_subtract(b: dynamic)
  {
      return Point((x - b.x), (y - b.y));
    }
  func dist()
  {
      return sqrt(((x * x) + (y * y)));
    }
  func operator_multiply(b: dynamic)
  {
      return ((x * b.y) - (y * b.x));
    }
  func operator_remainder(b: dynamic)
  {
      return ((x * b.x) + (y * b.y));
    }
  func operator_multiply(k: dynamic)
  {
      return Point((x * k), (y * k));
    }
  func norm()
  {
      var d = ((*this)).dist();
      x /= d;
      y /= d;
    }
  func print()
  {
      printf("%.4f %.4f\n", x, y);
    }
}

var a: dynamic;

func check(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  var p1 = (((b - a)) * ((c - a)));
  var p2 = (((b - a)) * ((d - a)));
  var p3 = (((d - c)) * ((a - c)));
  var p4 = (((d - c)) * ((b - c)));
  if ((fabs(p1) < eps))
  {
    p1 = 0.0;
  }
  if ((fabs(p2) < eps))
  {
    p2 = 0.0;
  }
  if ((fabs(p1) < eps))
  {
    p3 = 0.0;
  }
  if ((fabs(p1) < eps))
  {
    p4 = 0.0;
  }
  if ((((p1 * p2) < eps) && ((p3 * p4) < eps)))
  {
    return 1;
  }
  return 0;
}

func main()
{
  var s: dynamic;
  var e: dynamic;
  read(s.x, s.y);
  read(e.x, e.y);
  var c = (e - s);
  var l = c.dist();
  c.norm();
  var n: dynamic;
  read(n);
  a.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i].x, a[i].y);
      var f = ((a[i] - s));
      b[i] = (c * f);
      i += 1;
    }
  }
  var p1 = 0;
  var p2 = 0;
  var f = 1;
  {
    var i = 0;
    while ((i < n))
    {
      if (check(s, e, a[i], a[(((i + 1)) % n)]))
      {
        f = 0;
      }
      i += 1;
    }
  }
  if (f)
  {
    printf("%.10f", l);
    return 0;
  }
  f = 1;
  {
    var i = 0;
    while ((i < n))
    {
      if ((b[i] <= (-eps)))
      {
        f = 0;
      }
      i += 1;
    }
  }
  if (f)
  {
    printf("%.10f\n", l);
    return 0;
  }
  f = 1;
  {
    var i = 0;
    while ((i < n))
    {
      if ((b[i] >= eps))
      {
        f = 0;
      }
      i += 1;
    }
  }
  if (f)
  {
    printf("%.10f", l);
    return 0;
  }
  var mx = -1e18;
  var mn = 1e18;
  {
    var i = 0;
    while ((i < n))
    {
      var j = (((i + 1)) % n);
      var d = ((a[j] - a[i])).dist();
      if (((b[i] >= 0.0) && (b[j] >= 0)))
      {
        p1 += d;
      } else if (((b[i] < 0.0) && (b[j] < 0.0)))
      {
        p2 += d;
      } else if (((b[i] > (-eps)) && (b[j] < eps)))
      {
        var v = fabs(b[i]);
        var u = fabs(b[j]);
        p1 += ((v * d) / ((v + u)));
        p2 += ((u * d) / ((v + u)));
        var x = ((((u * ((a[i] % c))) + (v * ((a[j] % c))))) / ((v + u)));
        mx = max(mx, x);
        mn = min(mn, x);
      } else
      {
        var v = fabs(b[i]);
        var u = fabs(b[j]);
        p2 += ((v * d) / ((v + u)));
        p1 += ((u * d) / ((v + u)));
        var x = ((((u * ((a[i] % c))) + (v * ((a[j] % c))))) / ((v + u)));
        mx = max(mx, x);
        mx = max(mx, x);
        mn = min(mn, x);
      }
      i += 1;
    }
  }
  var p = min(p1, p2);
  var h = (mx - mn);
  if ((p < eps))
  {
    h = 0.0;
  }
  printf("%.10f", max(l, ((min(p, (2 * h)) + l) - h)));
  return 0;
}

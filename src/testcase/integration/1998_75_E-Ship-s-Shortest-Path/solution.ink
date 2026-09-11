// Translated from solution.cpp.

var eps: dynamic = 1e-4;

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Point() -> dynamic
  {
      x = cpp_assign(y, "=", 0);
    }
  func Point(xx: dynamic, yy: dynamic) -> dynamic
  {
      x = xx;
      y = yy;
    }
  func operator_add(b: dynamic) -> dynamic
  {
      return Point((x + b.x), (y + b.y));
    }
  func operator_subtract(b: dynamic) -> dynamic
  {
      return Point((x - b.x), (y - b.y));
    }
  func dist() -> dynamic
  {
      return sqrt(((x * x) + (y * y)));
    }
  func operator_multiply(b: dynamic) -> dynamic
  {
      return ((x * b.y) - (y * b.x));
    }
  func operator_remainder(b: dynamic) -> dynamic
  {
      return ((x * b.x) + (y * b.y));
    }
  func operator_multiply(k: dynamic) -> dynamic
  {
      return Point((x * k), (y * k));
    }
  func norm() -> dynamic
  {
      var d: dynamic = ((*self)).dist();
      x /= d;
      y /= d;
    }
  func print() -> dynamic
  {
      printf("%.4f %.4f\n", x, y);
    }
}

var a: dynamic = cpp_uninitialized();

func check(a: dynamic, b: dynamic, c: dynamic, d: dynamic) -> dynamic
{
  var p1: dynamic = (((b - a)) * ((c - a)));
  var p2: dynamic = (((b - a)) * ((d - a)));
  var p3: dynamic = (((d - c)) * ((a - c)));
  var p4: dynamic = (((d - c)) * ((b - c)));
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

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
  read(s.x, s.y);
  read(e.x, e.y);
  var c: dynamic = (e - s);
  var l: dynamic = c.dist();
  c.norm();
  var n: dynamic = cpp_uninitialized();
  read(n);
  a.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i].x, a[i].y);
      var f: dynamic = ((a[i] - s));
      b[i] = (c * f);
      i += 1;
    }
  }
  var p1: dynamic = 0;
  var p2: dynamic = 0;
  var f: dynamic = 1;
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
  var mx: dynamic = -1e18;
  var mn: dynamic = 1e18;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var j: dynamic = (((i + 1)) % n);
      var d: dynamic = ((a[j] - a[i])).dist();
      if (((b[i] >= 0.0) && (b[j] >= 0)))
      {
        p1 += d;
      } else if (((b[i] < 0.0) && (b[j] < 0.0)))
      {
        p2 += d;
      } else if (((b[i] > (-eps)) && (b[j] < eps)))
      {
        var v: dynamic = fabs(b[i]);
        var u: dynamic = fabs(b[j]);
        p1 += ((v * d) / ((v + u)));
        p2 += ((u * d) / ((v + u)));
        var x: dynamic = ((((u * ((a[i] % c))) + (v * ((a[j] % c))))) / ((v + u)));
        mx = max(mx, x);
        mn = min(mn, x);
      } else
      {
        var v: dynamic = fabs(b[i]);
        var u: dynamic = fabs(b[j]);
        p2 += ((v * d) / ((v + u)));
        p1 += ((u * d) / ((v + u)));
        var x: dynamic = ((((u * ((a[i] % c))) + (v * ((a[j] % c))))) / ((v + u)));
        mx = max(mx, x);
        mx = max(mx, x);
        mn = min(mn, x);
      }
      i += 1;
    }
  }
  var p: dynamic = min(p1, p2);
  var h: dynamic = (mx - mn);
  if ((p < eps))
  {
    h = 0.0;
  }
  printf("%.10f", max(l, ((min(p, (2 * h)) + l) - h)));
  return 0;
}
